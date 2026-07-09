#include "StaticModelStandardPerObjectDrawRequestBase.h"

void FWK::Graphics::StaticModelStandardPerObjectDrawRequestBase::BeginFrame()
{
	// 参照が途切れているstd::weak_ptrを削除する
	m_forwardDrawRequestPerObjectDataList.BeginFrame();
}

void FWK::Graphics::StaticModelStandardPerObjectDrawRequestBase::SetupPerObjectConstantBuffer(const Renderer& a_renderer, const RootSignature& a_rootSignature, const FrameResource& a_frameResource)
{
	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();

	// 描画処理を行うための定数バッファを送信していく
	for (const auto& l_drawRequest : m_forwardDrawRequestPerObjectDataList.GetREFArrayElementDataList())
	{
		const auto& l_drawRequestPerObject = l_drawRequest.m_type.lock();

		if (!l_drawRequestPerObject) { continue; }

		const auto& l_staticModelRecord = l_drawRequestPerObject->m_staticModelRecord.lock();

		FWK_ASSERT_RETURN_IF(!l_staticModelRecord, "StaticModelRecordのポインタが無効です。");

		const auto& l_modelData     = l_staticModelRecord->GetREFModelData();
		const float l_worldMaxScale = Utility::CalculateWorldMaxScale     (l_drawRequestPerObject->m_worldMatrix);

		for(const auto& l_modelMesh : l_modelData.m_modelMeshList)
		{
			// メッシュ単位で実行
			const auto& l_modelMeshletData         = l_modelMesh.m_modelMeshletData;
			const auto& l_modelMeshRuntimeData     = l_modelMesh.m_modelMeshRuntimeData;
			const auto& l_modelMaterialAssetData   = l_modelMesh.m_modelMaterial.m_modelMaterialAssetData;
			const auto& l_modelMaterialRuntimeData = l_modelMesh.m_modelMaterial.m_modelMaterialRuntimeData;

			FWK_ASSERT_RETURN_IF(l_modelMeshletData.m_meshletList.size() == StaticModelMeshletBuilder::k_emptyMeshletCount, "Meshletが存在しないため、StaticModelのPerObject定数バッファを設定できませんでした");

			Struct::CBStaticModelPerObject l_cbStaticModelPerObject = {};

			// モデル1体ごとのワールド行列
			l_cbStaticModelPerObject.m_worldMatrix = l_drawRequestPerObject->m_worldMatrix;

			// BackfaceConeCullingでconeAxisをWorld空間へ変換するための行列
			// coneAxisは位置ではなく向きなので、法線と同じく逆行列の転置で変換する
			l_cbStaticModelPerObject.m_worldInverseTransposeMatrix = l_drawRequestPerObject->m_worldInverseTransposeMatrix;

			// MehsletBoundsのradiusをWorld空間へ変換するための最大スケール
			l_cbStaticModelPerObject.m_worldMaxScale = l_worldMaxScale;

			// Material係数
			l_cbStaticModelPerObject.m_baseColorFactor = l_modelMaterialAssetData.m_baseColorFactor;
			l_cbStaticModelPerObject.m_metallicFactor  = l_modelMaterialAssetData.m_metallicFactor;
			l_cbStaticModelPerObject.m_roughnessFactor = l_modelMaterialAssetData.m_roughnessFactor;

			// MeshShaderで参照するStructuredBufferのSRV番号
			l_cbStaticModelPerObject.m_vertexBufferSRVDescriptorIndex            = l_modelMeshRuntimeData.m_vertexBuffer.GetVALSRVDescriptorIndex           ();
			l_cbStaticModelPerObject.m_meshletBufferSRVDescriptorIndex           = l_modelMeshRuntimeData.m_meshletBuffer.GetVALSRVDescriptorIndex          ();
			l_cbStaticModelPerObject.m_uniqueVertexIndexBufferSRVDescriptorIndex = l_modelMeshRuntimeData.m_uniqueVertexIndexBuffer.GetVALSRVDescriptorIndex();
			l_cbStaticModelPerObject.m_primitiveIndexBufferSRVDescriptorIndex    = l_modelMeshRuntimeData.m_primitiveIndexBuffer.GetVALSRVDescriptorIndex   ();
			l_cbStaticModelPerObject.m_meshletBoundsBufferSRVDescriptorIndex     = l_modelMeshRuntimeData.m_meshletBoundsBuffer.GetVALSRVDescriptorIndex    ();

			FWK_ASSERT_RETURN_IF(l_cbStaticModelPerObject.m_vertexBufferSRVDescriptorIndex		      == DescriptorHeap::k_invalidDescriptorIndex, "VertexBufferのSRVDescriptorIndexが無効です。");
			FWK_ASSERT_RETURN_IF(l_cbStaticModelPerObject.m_meshletBufferSRVDescriptorIndex		      == DescriptorHeap::k_invalidDescriptorIndex, "MeshletBufferのSRVDescriptorIndexが無効です。");
			FWK_ASSERT_RETURN_IF(l_cbStaticModelPerObject.m_uniqueVertexIndexBufferSRVDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "UniqueVertexIndexBufferのSRVDescriptorIndexが無効です。");
			FWK_ASSERT_RETURN_IF(l_cbStaticModelPerObject.m_primitiveIndexBufferSRVDescriptorIndex    == DescriptorHeap::k_invalidDescriptorIndex, "PrimitiveIndexBufferのSRVDescriptorIndexが無効です。");
			FWK_ASSERT_RETURN_IF(l_cbStaticModelPerObject.m_meshletBoundsBufferSRVDescriptorIndex     == DescriptorHeap::k_invalidDescriptorIndex, "MeshletBoundsBufferのSRVDescriptorIndexが無効です。");

			const auto l_baseColorTextureSRVDescriptorIndex = FetchTextureSRVDescriptorIndex(l_modelMaterialRuntimeData.m_baseColorTexture);
			const auto l_normalTextureSRVDescriptorIndex    = FetchTextureSRVDescriptorIndex(l_modelMaterialRuntimeData.m_normalTexture);
			const auto l_metallicTextureSRVDescriptorIndex  = FetchTextureSRVDescriptorIndex(l_modelMaterialRuntimeData.m_metallicTexture);
			const auto l_roughnessTextureSRVDescriptorIndex = FetchTextureSRVDescriptorIndex(l_modelMaterialRuntimeData.m_roughnessTexture);

			FWK_ASSERT_RETURN_IF(l_baseColorTextureSRVDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "BaseColorTextureのSRVDescriptorIndexが無効です。");
			FWK_ASSERT_RETURN_IF(l_normalTextureSRVDescriptorIndex    == DescriptorHeap::k_invalidDescriptorIndex, "NormalTextureのSRVDescriptorIndexが無効です。");
			FWK_ASSERT_RETURN_IF(l_metallicTextureSRVDescriptorIndex  == DescriptorHeap::k_invalidDescriptorIndex, "MetallicTextureのSRVDescriptorIndexが無効です。");
			FWK_ASSERT_RETURN_IF(l_roughnessTextureSRVDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "RoughnessのSRVDescriptorIndexが無効です。");

			l_cbStaticModelPerObject.m_baseColorTextureSRVDescriptorIndex = l_baseColorTextureSRVDescriptorIndex;
			l_cbStaticModelPerObject.m_normalTextureSRVDescriptorIndex    = l_normalTextureSRVDescriptorIndex;
			l_cbStaticModelPerObject.m_metallicTextureSRVDescriptorIndex  = l_metallicTextureSRVDescriptorIndex;
			l_cbStaticModelPerObject.m_roughnessTextureSRVDescriptorIndex = l_roughnessTextureSRVDescriptorIndex;

			// モデル描画用定数のセット
			SetupConstantBuffer<StaticModelPerObjectDynamicConstantBufferUploader>(l_cbStaticModelPerObject,
																			       a_rootSignature,
																			       l_directCommandList,
																			       a_frameResource,
																			       Enum::RootParameterType::CBStaticModelPerObject);	

			const bool l_isDispatchModelMeshSuccess = DispatchModelMesh(l_directCommandList, l_modelMesh);

			FWK_ASSERT_RETURN_IF(!l_isDispatchModelMeshSuccess, "StaticModelのMeshlet描画に失敗しました。");
		}
	}
}

void FWK::Graphics::StaticModelStandardPerObjectDrawRequestBase::AddDrawRequest(const std::shared_ptr<Struct::StaticModelStandardPerObjectDrawRequestData>& a_drawRequestData)
{
	FWK_ASSERT_RETURN_IF(!a_drawRequestData, "StaticModelStandardPerObjectDrawRequestDataが無効のため、描画申請の追加が出来ませんでした。");

	m_forwardDrawRequestPerObjectDataList.Add(a_drawRequestData);
}

bool FWK::Graphics::StaticModelStandardPerObjectDrawRequestBase::DispatchModelMesh(const DirectCommandList& a_directCommandList, const Graphics::StaticModelRecord::StaticModelMesh& a_modelMesh) const
{
	const auto l_meshletCount = static_cast<UINT>(a_modelMesh.m_modelMeshletData.m_meshletList.size());

	FWK_ASSERT_RETURN_VALUE_IF(l_meshletCount == StaticModelMeshletBuilder::k_emptyMeshletCount, "Meshletが存在しないため、StaticModelを描画できませんでした。", false);

	a_directCommandList.DispatchMesh(l_meshletCount, Constant::k_defaultDispatchMeshThreadGroupCountY, Constant::k_defaultDispatchMeshThreadGroupCountZ);

	return true;
}