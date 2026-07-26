#include "CascadeShadowMap.h"

void FWK::Graphics::CascadeShadowMap::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

bool FWK::Graphics::CascadeShadowMap::Create(const Device&                             a_device, 
	                                         const GPUMemoryAllocator&                 a_gpuMemoryAllocator, 
	                                               TypeAlias::DSVDescriptorPool&       a_dsvDescriptorPool,
	                                               TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool)
{
    // Cascade Shadow Mapは通常描画時にShaderから読み取るため、
	// SRVFormatが必須になる。
	FWK_ASSERT_RETURN_VALUE_IF(m_depthStencilTextureSettings.m_srvFormat == DXGI_FORMAT_UNKNOWN,     "CascadeShadowMapのSRVFormatが無効のため、作成処理に失敗しました。",        false);
	FWK_ASSERT_RETURN_VALUE_IF(m_depthStencilTextureSettings.m_mipLevels != k_requiredMIPLevelCount, "CascadeShadowMapのMIPLevelsがOneではないため、作成処理に失敗しました。",   false);
	FWK_ASSERT_RETURN_VALUE_IF(m_depthStencilTextureSettings.m_sampleCount != k_requiredSampleCount, "CascadeShadowMapのSampleCountがOneではないため、作成処理に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF(m_depthStencilTextureSettings.m_arraySize == k_invalidCascadeCount,   "CascadeShadowMapのArraySizeがZeroのため、作成処理に失敗しました。",        false);

	// Texture2DArrayのSlics数と同じ数だけ、
	// Cascadeごとの計算結果を保存する領域を作成する
	m_cascadeDataList.resize(m_depthStencilTextureSettings.m_arraySize);

	// ShadowMapの解像度から、
	// Shadow描画専用のViewportとScissorRECTを設定する。
	FWK_ASSERT_RETURN_VALUE_IF(!m_renderArea.Setup(m_resolution, m_resolution), "CascadeShadowMap用RenderAreaの設定処理に失敗しました。", false);

	FWK_ASSERT_RETURN_VALUE_IF(!m_depthStencilTexture.Create(a_device,
			                                                 a_gpuMemoryAllocator,
			                                                 m_depthStencilTextureSettings,
			                                                 m_resolution,
			                                                 m_resolution,
			                                                 a_dsvDescriptorPool,
			                                                 a_cbvSRVUAVDescriptorPool),
		                                                     "CascadeShadowMap用DepthStencilTextureの作成処理に失敗しました。",
		                                                     false);

	return true;
}

bool FWK::Graphics::CascadeShadowMap::Update()
{
	const auto& l_cbCameraPass = m_cbCameraPass.lock();
	const auto& l_cbLightPass  = m_cbLightPass.lock ();

	FWK_ASSERT_RETURN_VALUE_IF(!l_cbCameraPass,           "CBCameraPassが無効なため、CascadeShadowMapの更新処理に失敗しました。",  false);
	FWK_ASSERT_RETURN_VALUE_IF(!l_cbLightPass,            "CBLightPassが無効なため、CascadeShadowMapの更新処理に失敗しました。",   false);
	FWK_ASSERT_RETURN_VALUE_IF(m_cascadeDataList.empty(), "CascadeDataListが空のため、CascadeShadowMapの更新処理に失敗しました。", false);

	FWK_ASSERT_RETURN_VALUE_IF(l_cbCameraPass->m_nearClip <= k_invalidClipDistance ||
		                       l_cbCameraPass->m_farClip <= l_cbCameraPass->m_nearClip,
		                       "CameraのNearClipまたはFarClipが無効なため、CascadeShadowMapの更新処理に失敗しました。",
		                       false);

	// Cameraが描画する奥行き方向全体の長さを取得
	const float l_clipRange = l_cbCameraPass->m_farClip - l_cbCameraPass->m_nearClip;

	// Logarithmic分割を計算するためのFar/Near比率。
	// CascadeShadowMapでは、Cameraに近い領域を細かく分割したほうが
	// ShadowMapの解像度を有効に使える
	const float l_clipRatio = l_cbCameraPass->m_farClip / l_cbCameraPass->m_nearClip;

	// CameraのViewProjection行列は、World座標をNDC座標へ変換する行列
	// その逆行列を使用するとNDC座標からWorld座標へ戻せる
	// この逆行列を使用して、CameraFrustumの8つのCornerをWorld空間へ復元する
	const auto& l_inverseViewProjectionMatrix = l_cbCameraPass->m_viewProjectionMatrix.Invert();

	// NDC空間で定義されているCameraFrustumの8つのCornerを、
	// World空間へ変換した結果を保存する
	std::array<TypeAlias::Math::Vector3, k_frustumCornerCount> l_worldFrustumCornerList = {};

	// Camera FrustumのNearPlaneとFarPlaneを構成する
	// 8つすべてのCornerをNDC空間からWorld空間へ戻す
	for (std::size_t l_cornerIndex = 0ULL; l_cornerIndex < k_frustumCornerCount; ++l_cornerIndex)
	{
		const auto& l_ndcCorner = DirectX::XMLoadFloat3(&k_ndcFrustumCornerList[l_cornerIndex]);

		// XMVector3TransformCoordは行列変換後にW除算まで行う
		// そのため、射影変換後のNDC座標をWorld座標へ戻せる
		const auto& l_worldCorner = DirectX::XMVector3TransformCoord(l_ndcCorner, l_inverseViewProjectionMatrix);

		DirectX::XMStoreFloat3(&l_worldFrustumCornerList[l_cornerIndex], l_worldCorner);
	}

	// Directional LightのDirectionは、
	// 光が進んでいく方向を表す。
	// LightCameraは、この方向とは反対側に配置して
	// Cascadeの中心を見るようにする
	auto l_lightDirection = l_cbLightPass->m_directionalLight.m_direction;

	// 長さがZeroに近いDirectionは正規化できず、
	// LightView行列も作成できない
	FWK_ASSERT_RETURN_VALUE_IF(l_lightDirection.LengthSquared() <= k_directionLengthSquaredEpsilon, "DirectionalLightのDirectionがZeroに近いため、CascadeShadowMapの更新処理に失敗しました。", false);

	// LightCameraの配置方向として使用するため、
	// Directionの長さを正規化する
	l_lightDirection.Normalize();

	// LightView行列を作成する際の上方向には、
	// 基本的にWorld空間のY軸を使用する
	auto l_lightUp = TypeAlias::Math::Vector3::Up;

	// DirectionとUpがほぼ平行だと、
	// LookAt行列のRight方向を安定して計算できない
	// その場合はWorld空間のZ軸をUpとして使用する
	if (const float l_lightDirectionUpDot = std::abs(l_lightDirection.Dot(l_lightUp));
		l_lightDirectionUpDot >= k_parallelUpDotThreshold)
	{
		l_lightUp = TypeAlias::Math::Vector3::Backward;
	}

	// 現在作成されているCascade数を取得する。
    // Cascadeの番号をCameraFrustum全体に対する
    // Zeroより大きくOne以下の割合へ変換する際に使用する。
	const float l_cascadeCount = static_cast<float>(m_cascadeDataList.size());

	// 最初のCascadeはCameraのNearPlaneから始まる
	// Camera Frustum全体におけるNearPlaneの位置は、
	// k_initialSplitRatioとして扱う
	float l_previousSplitRatio = k_initialSplitRatio;

	// CameraFrustumをCascadeごとに分割し、
	// 各CascadeをLight視点で描画するための行列を計算する
	for (std::size_t l_cascadeIndex = 0ULL; l_cascadeIndex < m_cascadeDataList.size(); ++l_cascadeIndex)
	{
		// 現在のCascadeが全Cascade中のどこにあるかを、0.0F ~ 1.0Fの割合で求める
		const float l_cascadePosition = static_cast<float>(l_cascadeIndex + k_cascadeNumberOffset) / l_cascadeCount;

		// Logarithmic分割によるCascadeの終端距離
		// Cameraに近い部分を細かく、
		// 遠い部分を広く分割する。
		// これにより、見た目への影響が大きい近距離へ
		// ShadowMapの解像度を多く割り当てられる
		const float l_logarithmicSplitDepth = l_cbCameraPass->m_nearClip * std::pow(l_clipRatio, l_cascadePosition);

		// Uniform分割によるCascadeの終端距離。
		// CameraのNearClipからFarClipまでを、
		// 完全に等間隔で分割する
		const float l_uniformSplitDepth = l_cbCameraPass->m_nearClip + l_clipRange * l_cascadePosition;

		// Practical SplitSchemeを使用して、
		// Logarithmic分割とUniform分割を混ぜる
		// k_cascadeSplitLambdaが0.0Fに近いほどUniform寄り、
		// 1.0Fに近いほどLogarithmic寄りになる
		const float l_splitDepth = k_cascadeSplitLambda * l_logarithmicSplitDepth + (k_fullSplitWeight - k_cascadeSplitLambda) * l_uniformSplitDepth;

		// 求めたCascade終端距離を
		// CameraFrustum全体における割合へ変換する
		// この割合を使って、全体FrustumのNearCornerから
		// FarCornerまでの線分を補間する
		const float l_currentSplitRatio = (l_splitDepth - l_cbCameraPass->m_nearClip) / l_clipRange;

		// 現在のCascadeが担当する範囲だけを切り出した、
		// World空間のFrustum Cornerを保存する。
		std::array<TypeAlias::Math::Vector3, k_frustumCornerCount> l_cascadeFrustumCornerList = {};

		// 全体FrustumのNearPlaneとFarPlaneの対応するCornerを使い、
		// 現在のCascadeの開始位置と終了位置を補間して求める。
		for (std::size_t l_cornerIndex = 0ULL; l_cornerIndex < k_frustumPlaneCornerCount; ++l_cornerIndex)
		{
			const auto& l_nearCorner = l_worldFrustumCornerList[l_cornerIndex];
			const auto& l_farCorner = l_worldFrustumCornerList[l_cornerIndex + k_frustumPlaneCornerCount];

			// CameraFrustum全体のNearCornerから
			// 対応するFarCornerへ向かうベクトル
			const auto l_nearToFar = l_farCorner - l_nearCorner;

			// 現在のCascadeのNear側Corner。
			// 最初のCascadeではCameraのNearPlane、
			// それ以降は一つ前のCascadeの終端位置になる
			l_cascadeFrustumCornerList[l_cornerIndex] = l_nearCorner + l_nearToFar * l_previousSplitRatio;

			// 現在のCascadeのFar側Corner
			l_cascadeFrustumCornerList[l_cornerIndex + k_frustumPlaneCornerCount] = l_nearCorner + l_nearToFar * l_currentSplitRatio;
		}

		// 現在のCascadeの8つのCornerを合計し、
		// その平均からCascade領域の中心を求める。
		// この中心をLightCameraが見るTargetとして使用する。
		auto l_cascadeCenter = TypeAlias::Math::Vector3::Zero;

		for (const auto& l_corner : l_cascadeFrustumCornerList)
		{
			l_cascadeCenter += l_corner;
		}

		l_cascadeCenter /= static_cast<float>(k_frustumCornerCount);

		// Cascade中心から最も遠いCornerまでの距離を求める
		// この値はCascade Frustum全体を囲む球の半径として扱える
		float l_cascadeRadius = k_initialRadius;

		for (const auto& l_corner : l_cascadeFrustumCornerList)
		{
			const float l_cornerDistance = (l_corner - l_cascadeCenter).Length();

			l_cascadeRadius = std::max(l_cascadeRadius, l_cornerDistance);
		}

		// Light CameraをCascade中心から離す距離を求める
		// Cascadeの半径だけではDepth方向の余裕が不足するため、
		// ScaleとPaddingを加えて十分な距離を確保する
		const float l_lightViewDistance = l_cascadeRadius * k_lightViewDistanceScale + k_lightViewDepthPadding;

		// 光が進む方向とは逆側へLightCameraを配置する
		// これにより、LightCameraからCascade中心を見る方向と
		// DirectionalLightが光を照射する方向が一致する
		const auto& l_lightPosition = l_cascadeCenter - l_lightDirection * l_lightViewDistance;

		// World空間をLightView空間へ変換するView行列を作成する
		const auto& l_lightViewMatrix = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&l_lightPosition), DirectX::XMLoadFloat3(&l_cascadeCenter), DirectX::XMLoadFloat3(&l_lightUp));

		// Light View空間へ変換したCascadeFrustumを囲む
		// AxisAlignedBoundingBoxの最小座標。
		TypeAlias::Math::Vector3 l_min = { std::numeric_limits<float>::max(),
		                                   std::numeric_limits<float>::max(),
		                                   std::numeric_limits<float>::max() };

		TypeAlias::Math::Vector3 l_max = { std::numeric_limits<float>::lowest(),
		                                   std::numeric_limits<float>::lowest(),
		                                   std::numeric_limits<float>::lowest() };
		
		// Cascadeの8つのCornerをLight View空間へ変換し、
		// そのすべてを収める最小・最大範囲を求める
		// X・Y範囲 : Orthographic Projectionの横幅と縦幅
		// Z範囲    : Orthographic ProjectionのNearとFar
		for (const auto& l_corner : l_cascadeFrustumCornerList)
		{
			const auto& l_lightViewCornerVector = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&l_corner), l_lightViewMatrix);
			      auto  l_lightViewCorner       = TypeAlias::Math::Vector3::Zero;

			DirectX::XMStoreFloat3(&l_lightViewCorner, l_lightViewCornerVector);

			l_min.x = std::min(l_min.x, l_lightViewCorner.x);
			l_min.y = std::min(l_min.y, l_lightViewCorner.y);
			l_min.z = std::min(l_min.z, l_lightViewCorner.z);

			l_max.x = std::max(l_max.x, l_lightViewCorner.x);
			l_max.y = std::max(l_max.y, l_lightViewCorner.y);
			l_max.z = std::max(l_max.z, l_lightViewCorner.z);
		}

		// Camera Frustumの外側に存在するShadowCasterも
		// Shadow Mapへ描画できるように、Depth方向へ余白を加える
		// 例えば、Camera Frustumの少し外側にある建物が、
		// Camera内の地面へ影を落とす場合がある
		l_min.z -= k_lightViewDepthPadding;
		l_max.z += k_lightViewDepthPadding;

		// Orthographic Projectionへ渡せる、
		// 有効な幅/高さ/奥行きがあるか検証する
		FWK_ASSERT_RETURN_VALUE_IF(l_min.x >= l_max.x ||
			                       l_min.y >= l_max.y ||
			                       l_min.z >= l_max.z,
			                       "CascadeのOrthographic範囲が無効なため、CascadeShadowMapの更新処理に失敗しました。",
			                       false);

		// DirectionalLightは距離によって光の広がりが変化しないため、
		// PerspectiveProjectionではなくOrthographicProjectionを使う
		// LightView空間で求めた最小・最大範囲を、
		// そのまま投影範囲へ設定する
		const auto& l_lightProjectionMatrix = DirectX::XMMatrixOrthographicOffCenterLH(l_min.x, 
			                                                                           l_max.x, 
			                                                                           l_min.y,
			                                                                           l_max.y,
			                                                                           l_min.z,
			                                                                           l_max.z);

		auto& l_cascadeData = m_cascadeDataList[l_cascadeIndex];

		// World座標をLightのClip空間へ変換する行列を保存する
		// Shadow描画時はモデル頂点をShadowMap上へ変換し、
		// 通常のLit描画時はWorld座標からShadowMap座標を求める
		l_cascadeData.m_viewProjectionMatrix = l_lightViewMatrix * l_lightProjectionMatrix;

		// 現在のCascadeがCameraからどの距離までを担当するか保存する
		// 後のLitShaderでPixelのView空間Depthと比較し、
		// 使用するCascadeを選択する。
		l_cascadeData.m_splitDepth = l_splitDepth;

		// 次のCascadeは、現在のCascadeの終端から開始する
		l_previousSplitRatio = l_currentSplitRatio;
	}

	return true;
}

nlohmann::json FWK::Graphics::CascadeShadowMap::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

FWK::TypeAlias::DescriptorIndex FWK::Graphics::CascadeShadowMap::FetchVALCascadeDSVDescriptorIndex(const UINT a_cascadeIndex) const
{
	return m_depthStencilTexture.FetchVALDSVDescriptorIndex(a_cascadeIndex, k_shadowMapMIPSlice);
}

const FWK::TypeAlias::Math::Matrix* FWK::Graphics::CascadeShadowMap::FetchREFCascadeViewProjectionMatrix(const UINT a_cascadeIndex) const
{
	FWK_ASSERT_RETURN_VALUE_IF(a_cascadeIndex >= m_cascadeDataList.size(), "CascadeIndexがCascadeDataListの範囲外のため、ViewProjectionMatrixの取得に失敗しました。", nullptr);

	return &m_cascadeDataList[a_cascadeIndex].m_viewProjectionMatrix;
}