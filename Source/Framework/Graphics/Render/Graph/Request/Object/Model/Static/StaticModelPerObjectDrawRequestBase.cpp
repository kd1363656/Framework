#include "StaticModelPerObjectDrawRequestBase.h"

void FWK::Graphics::StaticModelPerObjectDrawRequestBase::BeginFrame()
{
	// 参照が途切れているstd::weak_ptrを削除する
	m_forwardDrawRequestDataSmartPointerVectorArray.RemoveExpiredElements();
}

void FWK::Graphics::StaticModelPerObjectDrawRequestBase::SetupPerObjectConstantBuffer(const Renderer& a_renderer, const RootSignature& a_rootSignature, const FrameResource& a_frameResource)
{
	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();

	// 描画処理を行うための定数バッファを送信していく
	for (const auto& l_drawRequestData : m_forwardDrawRequestDataSmartPointerVectorArray.GetREFArrayElementDataList())
	{
		const auto& l_drawRequest = l_drawRequestData.m_type.lock();

		if (!l_drawRequest) { continue; }

		const auto& l_staticModelRecord = l_drawRequest->m_staticModelRecord.lock();

		if (!l_staticModelRecord) { continue; }
		
		const auto& l_modelData            = l_staticModelRecord->GetREFModelData    ();
		const float l_worldMaxScale        = Utility::CalculateWorldMaxScale         (l_drawRequest->m_worldMatrix);
		const float l_worldOrientationSign = l_drawRequest->m_worldMatrix.Determinant() < Constant::k_modelWorldOrientationDeterminantBoundary ? Constant::k_mirrorModelWorldOrientationSign : Constant::k_normalModelWorldOrientationSign;

		for(const auto& l_modelMesh : l_modelData.m_modelMeshList)
		{
			// メッシュ単位で実行
			const auto& l_modelMeshletData         = l_modelMesh.m_modelMeshletData;
			const auto& l_modelMeshRuntimeData     = l_modelMesh.m_modelMeshRuntimeData;
			const auto& l_modelMaterialAssetData   = l_modelMesh.m_modelMaterial.m_modelMaterialAssetData;
			const auto& l_modelMaterialRuntimeData = l_modelMesh.m_modelMaterial.m_modelMaterialRuntimeData;

			FWK_ASSERT_RETURN_IF(l_modelMeshletData.m_meshletList.empty(), "Meshletが存在しないため、StaticModelのPerObject定数バッファを設定できませんでした");

			Struct::CBModelPerObject l_cbModelPerObject = {};

			const auto l_meshletCount = static_cast<UINT>(l_modelMeshletData.m_meshletList.size());

			// モデル1体ごとのワールド行列
			l_cbModelPerObject.m_worldMatrix = l_drawRequest->m_worldMatrix;

			// BackfaceConeCullingでconeAxisをWorld空間へ変換するための行列
			// coneAxisは位置ではなく向きなので、法線と同じく逆行列の転置で変換する
			l_cbModelPerObject.m_worldInverseTransposeMatrix = l_drawRequest->m_worldInverseTransposeMatrix;

			// MehsletBoundsのradiusをWorld空間へ変換するための最大スケール
			l_cbModelPerObject.m_worldMaxScale = l_worldMaxScale;

			// ObjectのWorldTransformがMirror状態なら-1
			// 通常なら+1をシェーダーに送る
			l_cbModelPerObject.m_worldOrientationSign = l_worldOrientationSign;

			// 最後のAmplificationShaderGroupには、実際のMeshlet数を超えるthreadが含まれる可能性がある、
			// AS側はこの値を使用して範囲外のThreadを除外する
			l_cbModelPerObject.m_meshletCount = l_meshletCount;

			// Material係数
			l_cbModelPerObject.m_baseColorFactor = l_modelMaterialAssetData.m_baseColorFactor;
			l_cbModelPerObject.m_metallicFactor  = l_modelMaterialAssetData.m_metallicFactor;
			l_cbModelPerObject.m_roughnessFactor = l_modelMaterialAssetData.m_roughnessFactor;

			// MeshShaderで参照するStructuredBufferのSRV番号
			l_cbModelPerObject.m_vertexBufferSRVDescriptorIndex            = l_modelMeshRuntimeData.m_vertexBuffer.GetVALSRVDescriptorIndex           ();
			l_cbModelPerObject.m_meshletBufferSRVDescriptorIndex           = l_modelMeshRuntimeData.m_meshletBuffer.GetVALSRVDescriptorIndex          ();
			l_cbModelPerObject.m_uniqueVertexIndexBufferSRVDescriptorIndex = l_modelMeshRuntimeData.m_uniqueVertexIndexBuffer.GetVALSRVDescriptorIndex();
			l_cbModelPerObject.m_primitiveIndexBufferSRVDescriptorIndex    = l_modelMeshRuntimeData.m_primitiveIndexBuffer.GetVALSRVDescriptorIndex   ();
			l_cbModelPerObject.m_meshletBoundsBufferSRVDescriptorIndex     = l_modelMeshRuntimeData.m_meshletBoundsBuffer.GetVALSRVDescriptorIndex    ();

			FWK_ASSERT_RETURN_IF(l_cbModelPerObject.m_vertexBufferSRVDescriptorIndex            == DescriptorHeap::k_invalidDescriptorIndex, "VertexBufferのSRVDescriptorIndexが無効です。");
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

			const bool l_isDispatchModelMeshSuccess = DispatchModelMesh(l_directCommandList, l_modelMesh);

			FWK_ASSERT_RETURN_IF(!l_isDispatchModelMeshSuccess, "StaticModelのMeshlet描画に失敗しました。");
		}
	}
}

void FWK::Graphics::StaticModelPerObjectDrawRequestBase::AddDrawRequest(const std::shared_ptr<Struct::StaticModelPerObjectDrawRequestData>& a_drawRequestData)
{
	FWK_ASSERT_RETURN_IF(!a_drawRequestData, "DrawRequestDataが無効のため、描画申請の追加が出来ませんでした。");

	m_forwardDrawRequestDataSmartPointerVectorArray.Add(a_drawRequestData);
}

bool FWK::Graphics::StaticModelPerObjectDrawRequestBase::DispatchModelMesh(const DirectCommandList& a_directCommandList, const Graphics::StaticModelRecord::ModelMesh& a_modelMesh) const
{
	const auto& l_modelMeshletList = a_modelMesh.m_modelMeshletData.m_meshletList;

	FWK_ASSERT_RETURN_VALUE_IF(l_modelMeshletList.empty(), "Meshletが存在しないため、StaticModelを描画できませんでした。", false);

	const auto l_meshletCount = static_cast<UINT>(l_modelMeshletList.size());

	// 1つのAmplificationShaderGroupが、
	// 32個のMeshletを並列に処理する
	auto l_amplificationShaderGroupCount = l_meshletCount / Constant::k_meshletCountPerAmplificationShaderGroup;
	
	// Meshlet数が32の倍数でない場合は、
	// 余ったMeshletを処理するGroupを1つ追加する
	if (const auto l_meshletCountRemainder = l_meshletCount % Constant::k_meshletCountPerAmplificationShaderGroup;
		l_meshletCountRemainder != Constant::k_noRemainder)
	{
		++l_amplificationShaderGroupCount; 
	}

	a_directCommandList.DispatchMesh(l_amplificationShaderGroupCount, k_defaultDispatchMeshThreadGroupCountY, k_defaultDispatchMeshThreadGroupCountZ);

	return true;
}