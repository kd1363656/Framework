#include "SkeletalAnimationModelStandardPerObjectDrawRequestBase.h"

void FWK::Graphics::SkeletalAnimationModelStandardPerObjectDrawRequestBase::BeginFrame()
{
	// DrawRequestDataの所有権が破棄されていた場合は、
	// 内部配列から期限切れのstd::weak_ptrを削除する
	m_forwardDrawRequestDataSmartPointerVectorArray.BeginFrame();
}

void FWK::Graphics::SkeletalAnimationModelStandardPerObjectDrawRequestBase::SetupPerObjectConstantBuffer(const Renderer & a_renderer, const RootSignature & a_rootSignature, const FrameResource & a_frameResource)
{
	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();

	for (const auto& l_drawRequestData : m_forwardDrawRequestDataSmartPointerVectorArray.GetREFArrayElementDataList())
	{
		const auto& l_drawRequest = l_drawRequestData.m_type.lock();

		// BeginFrame後に所有者が破棄される可能性もあるため、
		// 無効な描画申請は処理しない
		if (!l_drawRequest) { continue; }

		const auto& l_skeletalAnimationPlayer = l_drawRequest->m_skeletalAnimationPlayer.lock();

		FWK_ASSERT_RETURN_IF(!l_skeletalAnimationPlayer, "SkeletalAnimationPlayerが無効なため、Skeletal Animation Modelを描画できません。");

		const auto& l_skeletalAnimationModelRecord = l_skeletalAnimationPlayer->GetREFSkeletalAnimationModelRecord().lock();

		FWK_ASSERT_RETURN_IF(!l_skeletalAnimationModelRecord, "SkeletalAnimationModelRecordが無効なため、Skeletal Animation Modelを描画できません。");

		// FrameDataはこの関数内でのみ参照し、
		// メンバ変数などへ保持しない。
		// そのため、所有権を持たない一時参照として
		// 生ポインタを使用して問題ない。
		const auto* l_frameData = l_skeletalAnimationPlayer->FindPTRCurrentFrameData();

		FWK_ASSERT_RETURN_IF(!l_frameData, "現在FrameDataを取得できないため、Skeletal Animation Modelを描画できません。");

		const auto& l_modelData               = l_skeletalAnimationModelRecord->GetREFModelData();
		const auto& l_modelMeshList           = l_modelData.m_modelMeshList;
		const auto& l_skinnedVertexBufferList = l_frameData->m_skinnedVertexBufferList;
		const auto& l_meshletBoundsBufferList = l_frameData->m_meshletBoundsBufferList;

		// Player::CreateではModel Mesh一つにつき、
		// 動的Meshlet BoundsBufferを一つ作成している。
		FWK_ASSERT_RETURN_IF(l_modelMeshList.size() != l_meshletBoundsBufferList.size(), "ModelMeshListとMeshletBoundsBufferListの要素数が一致しません。");
		FWK_ASSERT_RETURN_IF(l_modelMeshList.empty(),                                    "ModelMeshListが空のため、SkeletalAnimationModelを描画できません。");

		// Player::CreateではModel Mesh一個につき、
		// SkinnedVertexBufferを一個作成している
		FWK_ASSERT_RETURN_IF(l_modelMeshList.size() != l_skinnedVertexBufferList.size(), "ModelMeshListとSkinnedVertexBufferListの要素数が一致しません。");

		const float l_worldMaxScale = Utility::CalculateWorldMaxScale(l_drawRequest->m_worldMatrix);

		for (auto l_modelMeshIndex = 0ULL; l_modelMeshIndex < l_modelMeshList.size(); ++l_modelMeshIndex)
		{
			const auto& l_modelMesh                = l_modelMeshList[l_modelMeshIndex];
			const auto& l_modelMeshletData         = l_modelMesh.m_modelMeshletData;
			const auto& l_modelMeshRuntimeData     = l_modelMesh.m_modelMeshRuntimeData;
			const auto& l_modelMaterialAssetData   = l_modelMesh.m_modelMaterial.m_modelMaterialAssetData;
			const auto& l_modelMaterialRuntimeData = l_modelMesh.m_modelMaterial.m_modelMaterialRuntimeData;
			const auto& l_skinnedVertexBuffer      = l_skinnedVertexBufferList[l_modelMeshIndex];
			const auto& l_meshletBoundsBuffer      = l_meshletBoundsBufferList[l_modelMeshIndex];

			FWK_ASSERT_RETURN_IF(l_modelMeshletData.m_meshletList.empty(), "Meshletが存在しないため、Skeletal Animation Modelを描画できません。");

			// Compute Shaderは処理完了後に、
			// SkinnedVertexBufferをNON_PIXEL_SHADER_RESOURCEへ遷移している
			// Mesh ShaderはNON_PIXEL Shaderに含まれるため、
			// この状態でStructuredBufferとして読み取れる
			FWK_ASSERT_RETURN_IF(l_skinnedVertexBuffer.GetVALCurrentResourceState() != D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, "SkinnedVertexBufferがMesh Shaderから読み取れるResource Stateではありません。");
			FWK_ASSERT_RETURN_IF(l_meshletBoundsBuffer.GetVALElementCount() != l_modelMeshletData.m_meshletList.size(),                "MeshletBoundsBufferとModelMeshletListの要素数が一致しません。");
			FWK_ASSERT_RETURN_IF(l_meshletBoundsBuffer.GetVALElementCount() != l_modelMeshletData.m_meshletList.size(),                "MeshletBoundsBufferとModelMeshletListの要素数が一致しません。");

			// スキニング後の頂点構造はStatic Modelの頂点構造と同じため、
			// 既存のModel描画用定数バッファを共有する
			Struct::CBModelPerObject l_cbModelPerObject = {};

			// 必要なパラメータを設定していく
			l_cbModelPerObject.m_worldMatrix                 = l_drawRequest->m_worldMatrix;
			l_cbModelPerObject.m_worldInverseTransposeMatrix = l_drawRequest->m_worldInverseTransposeMatrix;
			l_cbModelPerObject.m_worldMaxScale               = l_worldMaxScale;
			l_cbModelPerObject.m_baseColorFactor             = l_modelMaterialAssetData.m_baseColorFactor;
			l_cbModelPerObject.m_metallicFactor              = l_modelMaterialAssetData.m_metallicFactor;
			l_cbModelPerObject.m_roughnessFactor             = l_modelMaterialAssetData.m_roughnessFactor;
			
			// Static Modelでは元頂点Bufferを設定するが、
			// Skeletal AnimationではComputeShaderが出力した
			// SkinnedVertexBufferのSRVを設定する
			l_cbModelPerObject.m_vertexBufferSRVDescriptorIndex            = l_skinnedVertexBuffer.GetVALSRVDescriptorIndex                           ();
			l_cbModelPerObject.m_meshletBufferSRVDescriptorIndex           = l_modelMeshRuntimeData.m_meshletBuffer.GetVALSRVDescriptorIndex          ();
			l_cbModelPerObject.m_uniqueVertexIndexBufferSRVDescriptorIndex = l_modelMeshRuntimeData.m_uniqueVertexIndexBuffer.GetVALSRVDescriptorIndex();
			l_cbModelPerObject.m_primitiveIndexBufferSRVDescriptorIndex    = l_modelMeshRuntimeData.m_primitiveIndexBuffer.GetVALSRVDescriptorIndex   ();
			l_cbModelPerObject.m_meshletBoundsBufferSRVDescriptorIndex     = l_meshletBoundsBuffer.GetVALSRVDescriptorIndex                           ();

			FWK_ASSERT_RETURN_IF(l_cbModelPerObject.m_vertexBufferSRVDescriptorIndex            == DescriptorHeap::k_invalidDescriptorIndex, "SkinnedVertexBufferのSRVDescriptorIndexが無効です。");
			FWK_ASSERT_RETURN_IF(l_cbModelPerObject.m_meshletBufferSRVDescriptorIndex           == DescriptorHeap::k_invalidDescriptorIndex, "MeshletBufferのSRVDescriptorIndexが無効です。");
			FWK_ASSERT_RETURN_IF(l_cbModelPerObject.m_uniqueVertexIndexBufferSRVDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "UniqueVertexIndexBufferのSRVDescriptorIndexが無効です。");
			FWK_ASSERT_RETURN_IF(l_cbModelPerObject.m_primitiveIndexBufferSRVDescriptorIndex    == DescriptorHeap::k_invalidDescriptorIndex, "PrimitiveIndexBufferのSRVDescriptorIndexが無効です。");
			FWK_ASSERT_RETURN_IF(l_cbModelPerObject.m_meshletBoundsBufferSRVDescriptorIndex     == DescriptorHeap::k_invalidDescriptorIndex, "MeshletBoundsBufferのSRVDescriptorIndexが無効です。");

			const auto l_baseColorTextureSRVDescriptorIndex = FetchTextureSRVDescriptorIndex(l_modelMaterialRuntimeData.m_baseColorTexture);
			const auto l_normalTextureSRVDescriptorIndex    = FetchTextureSRVDescriptorIndex(l_modelMaterialRuntimeData.m_normalTexture);
			const auto l_metallicTextureSRVDescriptorIndex  = FetchTextureSRVDescriptorIndex(l_modelMaterialRuntimeData.m_metallicTexture);
			const auto l_roughnessTextureSRVDescriptorIndex = FetchTextureSRVDescriptorIndex(l_modelMaterialRuntimeData.m_roughnessTexture);

			FWK_ASSERT_RETURN_IF(l_baseColorTextureSRVDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "BaseColorTextureのSRVDescriptorIndexが無効です。");
			FWK_ASSERT_RETURN_IF(l_normalTextureSRVDescriptorIndex    == DescriptorHeap::k_invalidDescriptorIndex, "NormalTextureのSRVDescriptorIndexが無効です。");
			FWK_ASSERT_RETURN_IF(l_metallicTextureSRVDescriptorIndex  == DescriptorHeap::k_invalidDescriptorIndex, "MetallicTextureのSRVDescriptorIndexが無効です。");
			FWK_ASSERT_RETURN_IF(l_roughnessTextureSRVDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "RoughnessのSRVDescriptorIndexが無効です。");

			l_cbModelPerObject.m_baseColorTextureSRVDescriptorIndex = l_baseColorTextureSRVDescriptorIndex;
			l_cbModelPerObject.m_normalTextureSRVDescriptorIndex    = l_normalTextureSRVDescriptorIndex;
			l_cbModelPerObject.m_metallicTextureSRVDescriptorIndex  = l_metallicTextureSRVDescriptorIndex;
			l_cbModelPerObject.m_roughnessTextureSRVDescriptorIndex = l_roughnessTextureSRVDescriptorIndex;

			// モデル描画用定数のセット
			SetupConstantBuffer<ModelPerObjectDynamicConstantBufferUploader>(l_cbModelPerObject,
																			 a_rootSignature,
																			 l_directCommandList,
																			 a_frameResource,
																			 Enum::RootParameterType::CBModelPerObject);

			// 現在Meshの定数バッファを設定し終えたため、
			// Meshが保持するMeshlet数だけMesh Shaderを起動する
			const bool l_isDispatchModelMeshSuccess = DispatchModelMesh(l_directCommandList, l_modelMesh);

			FWK_ASSERT_RETURN_IF(!l_isDispatchModelMeshSuccess, "Skeletal Animation ModelのMeshlet描画に失敗しました。");
		}
	}
}

void FWK::Graphics::SkeletalAnimationModelStandardPerObjectDrawRequestBase::AddDrawRequest(const std::shared_ptr<DrawRequestData>&a_drawRequestData)
{
	FWK_ASSERT_RETURN_IF(!a_drawRequestData, "DrawRequestDataが無効なため、Skeletal Animation Modelの描画申請を追加できません。");

	m_forwardDrawRequestDataSmartPointerVectorArray.Add(a_drawRequestData);
}

bool FWK::Graphics::SkeletalAnimationModelStandardPerObjectDrawRequestBase::DispatchModelMesh(const DirectCommandList& a_directCommandList, const SkeletalAnimationModelRecord::ModelMesh& a_modelMesh) const
{
	const auto& l_modelMeshletList = a_modelMesh.m_modelMeshletData.m_meshletList;

	FWK_ASSERT_RETURN_VALUE_IF(l_modelMeshletList.empty(), "Meshletが存在しないため、Skeletal Animation Modelを描画できません。", false);

	// DirectCommandList::DispatchMesh()が受け取るGroup数はUINT
	// Meshlet数はstd::size_tで保持されているため、
	// UINTへ変換する前に表現可能な範囲か確認する
	FWK_ASSERT_RETURN_VALUE_IF(l_modelMeshletList.size() > std::numeric_limits<UINT>::max(), "Meshlet数がUINTの最大値を超えているため、Skeletal Animation Modelを描画できません。", false);

	const auto l_meshletCount = static_cast<UINT>(l_modelMeshletList.size());

	a_directCommandList.DispatchMesh(l_meshletCount, k_defaultDispatchMeshThreadGroupCountY, k_defaultDispatchMeshThreadGroupCountZ);

	return true;
}