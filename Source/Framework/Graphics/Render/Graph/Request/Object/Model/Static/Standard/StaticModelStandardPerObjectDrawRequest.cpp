#include "StaticModelStandardPerObjectDrawRequest.h"

void FWK::Graphics::StaticModelStandardPerObjectDrawRequest::BeginFrame()
{
	// 参照が途切れているstd::weak_ptrを削除する
	m_forwardDrawRequestPerObjectDataList.BeginFrame();
}

void FWK::Graphics::StaticModelStandardPerObjectDrawRequest::SetupPerObjectConstantBuffer(const Renderer& a_renderer, const RootSignature& a_rootSignature, const FrameResource& a_frameResource)
{
	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();

	// 描画処理を行うための定数バッファを送信していく
	for (const auto& l_drawRequest : m_forwardDrawRequestPerObjectDataList.GetREFDrawRequestPerObjectRecordList())
	{
		const auto& l_drawRequestPerObject = l_drawRequest.m_drawRequestPerObject.lock();

		if (!l_drawRequestPerObject) { continue; }

		const auto& l_staticModelRecord = l_drawRequestPerObject->m_staticModelRecord.lock();

		FWK_ASSERT_RETURN_IF_FAILED(!l_staticModelRecord, "StaticModelRecordのポインタが無効です。");

		const auto& l_modelData = l_staticModelRecord->GetREFModelData();

		for(const auto& l_modelMesh : l_modelData.m_modelMeshList)
		{
			// メッシュ単位で実行
			const auto& l_modelMeshletData         = l_modelMesh.m_modelMeshletData;
			const auto& l_modelMeshRuntimeData     = l_modelMesh.m_modelMeshRuntimeData;
			const auto& l_modelMaterialAssetData   = l_modelMesh.m_modelMaterial.m_modelMaterialAssetData;
			const auto& l_modelMaterialRuntimeData = l_modelMesh.m_modelMaterial.m_modelMaterialRuntimeData;

			FWK_ASSERT_RETURN_IF_FAILED(l_modelMeshletData.m_meshletList.size() == Constant::k_emptyMeshletCount, "Meshletが存在しないため、StaticModelのPerObject定数バッファを設定できませんでした");

			Struct::CBStaticModelPerObject l_cbStaticModelPerObject = {};

			// モデル1体ごとのワールド行列
			l_cbStaticModelPerObject.m_worldMatrix = l_drawRequestPerObject->m_worldMatrix;

			// Material係数
			l_cbStaticModelPerObject.m_baseColorFactor = l_modelMaterialAssetData.m_baseColorFactor;
			l_cbStaticModelPerObject.m_metallicFactor  = l_modelMaterialAssetData.m_metallicFactor;
			l_cbStaticModelPerObject.m_roughnessFactor = l_modelMaterialAssetData.m_roughnessFactor;

			// MeshShaderで参照するStructuredBufferのSRV番号
			l_cbStaticModelPerObject.m_vertexBufferSRVDescriptorIndex            = l_modelMeshRuntimeData.m_vertexBuffer.m_srvDescriptorIndex;
			l_cbStaticModelPerObject.m_meshletBufferSRVDescriptorIndex           = l_modelMeshRuntimeData.m_meshletBuffer.m_srvDescriptorIndex;
			l_cbStaticModelPerObject.m_uniqueVertexIndexBufferSRVDescriptorIndex = l_modelMeshRuntimeData.m_uniqueVertexIndexBuffer.m_srvDescriptorIndex;
			l_cbStaticModelPerObject.m_primitiveIndexBufferSRVDescriptorIndex    = l_modelMeshRuntimeData.m_primitiveIndexBuffer.m_srvDescriptorIndex;
			l_cbStaticModelPerObject.m_meshletBoundsBufferSRVDescriptorIndex     = l_modelMeshRuntimeData.m_meshletBoundsBuffer.m_srvDescriptorIndex;

			FWK_ASSERT_RETURN_IF_FAILED(l_cbStaticModelPerObject.m_vertexBufferSRVDescriptorIndex		     == Constant::k_invalidDescriptorIndex, "VertexBufferのSRVDescriptorIndexが無効です。");
			FWK_ASSERT_RETURN_IF_FAILED(l_cbStaticModelPerObject.m_meshletBufferSRVDescriptorIndex		     == Constant::k_invalidDescriptorIndex, "MeshletBufferのSRVDescriptorIndexが無効です。");
			FWK_ASSERT_RETURN_IF_FAILED(l_cbStaticModelPerObject.m_uniqueVertexIndexBufferSRVDescriptorIndex == Constant::k_invalidDescriptorIndex, "UniqueVertexIndexBufferのSRVDescriptorIndexが無効です。");
			FWK_ASSERT_RETURN_IF_FAILED(l_cbStaticModelPerObject.m_primitiveIndexBufferSRVDescriptorIndex    == Constant::k_invalidDescriptorIndex, "PrimitiveIndexBufferのSRVDescriptorIndexが無効です。");
			FWK_ASSERT_RETURN_IF_FAILED(l_cbStaticModelPerObject.m_meshletBoundsBufferSRVDescriptorIndex     == Constant::k_invalidDescriptorIndex, "MeshletBoundsBufferのSRVDescriptorIndexが無効です。");

			const auto l_baseColorTextureSRVIndex  = FetchTextureSRVDescriptorIndex(l_modelMaterialRuntimeData.m_baseColorTexture);
			const auto l_normalTextureSRVIndex     = FetchTextureSRVDescriptorIndex(l_modelMaterialRuntimeData.m_normalTexture);
			const auto l_metallicTextureSRVIndex   = FetchTextureSRVDescriptorIndex(l_modelMaterialRuntimeData.m_metallicTexture);
			const auto l_roughenessTextureSRVIndex = FetchTextureSRVDescriptorIndex(l_modelMaterialRuntimeData.m_roughnessTexture);

			FWK_ASSERT_RETURN_IF_FAILED(l_baseColorTextureSRVIndex  == Constant::k_invalidDescriptorIndex, "BaseColorTextureのSRVDescriptorIndexが無効です。");
			FWK_ASSERT_RETURN_IF_FAILED(l_normalTextureSRVIndex     == Constant::k_invalidDescriptorIndex, "NormalTextureのSRVDescriptorIndexが無効です。");
			FWK_ASSERT_RETURN_IF_FAILED(l_metallicTextureSRVIndex   == Constant::k_invalidDescriptorIndex, "MetallicTextureのSRVDescriptorIndexが無効です。");
			FWK_ASSERT_RETURN_IF_FAILED(l_roughenessTextureSRVIndex == Constant::k_invalidDescriptorIndex, "RoughnessのSRVDescriptorIndexが無効です。");

			l_cbStaticModelPerObject.m_baseColorTextureSRVDescriptorIndex = l_baseColorTextureSRVIndex;
			l_cbStaticModelPerObject.m_normalTextureSRVDescriptorIndex    = l_normalTextureSRVIndex;
			l_cbStaticModelPerObject.m_metallicTextureSRVDescriptorIndex  = l_metallicTextureSRVIndex;
			l_cbStaticModelPerObject.m_roughnessTextureSRVDescriptorIndex = l_roughenessTextureSRVIndex;

			// モデル描画用定数のセット
			SetupConstantBuffer<StaticModelPerObjectConstantBufferUploader>(a_rootSignature, 
																			l_directCommandList,
																			a_frameResource,
																			l_cbStaticModelPerObject,
																			Enum::RootParameterType::CBStaticModelPerObject);	

			const bool l_isDispatchModelMeshSuccess = DispatchModelMesh(l_directCommandList, l_modelMesh);

			FWK_ASSERT_RETURN_IF_FAILED(!l_isDispatchModelMeshSuccess, "StaticModelのMeshlet描画に失敗しました。");
		}
	}
}

void FWK::Graphics::StaticModelStandardPerObjectDrawRequest::AddDrawRequest(const std::shared_ptr<Struct::StaticModelStandardPerObjectDrawRequestData>& a_drawRequestData)
{
	FWK_ASSERT_RETURN_IF_FAILED(!a_drawRequestData, "StaticModelStandardPerObjectDrawRequestDataが無効のため、描画申請の追加が出来ませんでした。");

	m_forwardDrawRequestPerObjectDataList.AddDrawRequestPerObject(a_drawRequestData);
}

bool FWK::Graphics::StaticModelStandardPerObjectDrawRequest::DispatchModelMesh(const DirectCommandList& a_directCommandList, const Struct::StaticModelMesh& a_modelMesh) const
{
	const auto l_meshletCount = static_cast<UINT>(a_modelMesh.m_modelMeshletData.m_meshletList.size());

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_meshletCount == Constant::k_emptyMeshletCount, "Meshletが存在しないため、StaticModelを描画できませんでした。", false);

	a_directCommandList.DispatchMesh(l_meshletCount, Constant::k_defaultDispatchMeshThreadGroupCountY, Constant::k_defaultDispatchMeshThreadGroupCountZ);

	return true;
}