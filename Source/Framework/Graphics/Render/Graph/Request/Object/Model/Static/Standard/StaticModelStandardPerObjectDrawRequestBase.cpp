#include "StaticModelStandardPerObjectDrawRequestBase.h"

void FWK::Graphics::StaticModelStandardPerObjectDrawRequestBase::BeginFrame()
{
	// 参照が途切れているstd::weak_ptrを削除する
	m_forwardDrawRequestPerObjectDataList.BeginFrame();
}

void FWK::Graphics::StaticModelStandardPerObjectDrawRequestBase::AddDrawRequest(const std::shared_ptr<Struct::StaticModelStandardPerObjectDrawRequestData>& a_drawRequestData)
{
	FWK_ASSERT_RETURN_IF_FAILED(!a_drawRequestData, "StaticModelStandardPerObjectDrawRequestDataが無効のため、描画申請の追加が出来ませんでした。");

	m_forwardDrawRequestPerObjectDataList.AddDrawRequestPerObject(a_drawRequestData);
}

void FWK::Graphics::StaticModelStandardPerObjectDrawRequestBase::SetupModelMeshConstantBuffer(const RootSignature&																   a_rootSignature,
																							  const DirectCommandList&															   a_directCommandList,
																							  const FrameResource&																   a_frameResource,
																							  const DrawRequestPerObjectList<Struct::StaticModelStandardPerObjectDrawRequestData>& a_drawRequestDataList,
																							  const TextureSystem&																   a_textureSystem)
{
	// 描画処理を行うための定数バッファを送信していく
	for (const auto& l_drawRequest : a_drawRequestDataList.GetREFDrawRequestPerObjectRecordList())
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
			const auto& l_modelMaterialRuntimeData = l_modelMesh.m_modelMaterial.m_modelMaterialRuntimeData;

			FWK_ASSERT_RETURN_IF_FAILED(l_modelMeshletData.m_meshletList.size() == Constant::k_emptyMeshletCount, "Meshletが存在しないため、StaticModelのPerObject定数バッファを設定できませんでした");

			Struct::CBStaticModelPerObject l_cbStaticModelPerObject = {};

			// モデル1体ごとのワールド行列
			l_cbStaticModelPerObject.m_worldMatrix = l_drawRequestPerObject->m_worldMatrix;

			// MeshShaderで参照するStructuredBufferのSRV番号
			l_cbStaticModelPerObject.m_vertexBufferSRVDescriptorIndex            = l_modelMeshRuntimeData.m_vertexBuffer.m_srvDescriptorIndex;
			l_cbStaticModelPerObject.m_meshletBufferSRVDescriptorIndex           = l_modelMeshRuntimeData.m_meshletBuffer.m_srvDescriptorIndex;
			l_cbStaticModelPerObject.m_uniqueVertexIndexBufferSRVDescriptorIndex = l_modelMeshRuntimeData.m_uniqueVertexIndexBuffer.m_srvDescriptorIndex;
			l_cbStaticModelPerObject.m_primitiveIndexBufferSRVDescriptorIndex    = l_modelMeshRuntimeData.m_primitiveIndexBuffer.m_srvDescriptorIndex;

			FWK_ASSERT_RETURN_IF_FAILED(l_cbStaticModelPerObject.m_vertexBufferSRVDescriptorIndex		     == Constant::k_invalidDescriptorIndex, "VertexBufferのSRVDescriptorIndexが無効です。");
			FWK_ASSERT_RETURN_IF_FAILED(l_cbStaticModelPerObject.m_meshletBufferSRVDescriptorIndex		     == Constant::k_invalidDescriptorIndex, "MeshletBufferのSRVDescriptorIndexが無効です。");
			FWK_ASSERT_RETURN_IF_FAILED(l_cbStaticModelPerObject.m_uniqueVertexIndexBufferSRVDescriptorIndex == Constant::k_invalidDescriptorIndex, "UniqueVertexIndexBufferのSRVDescriptorIndexが無効です。");
			FWK_ASSERT_RETURN_IF_FAILED(l_cbStaticModelPerObject.m_primitiveIndexBufferSRVDescriptorIndex    == Constant::k_invalidDescriptorIndex, "PrimitiveIndexBufferのSRVDescriptorIndexが無効です。");
		}
	}
}

bool FWK::Graphics::StaticModelStandardPerObjectDrawRequestBase::DispatchModelMesh(const DirectCommandList& a_directCommandList, const Struct::StaticModelMesh& a_modelMesh) const
{
	const auto l_meshletCount = static_cast<UINT>(a_modelMesh.m_modelMeshletData.m_meshletList.size());

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_meshletCount == Constant::k_emptyMeshletCount, "Meshletが存在しないため、StaticModelを描画できませんでした。", false);

	a_directCommandList.DispatchMesh(l_meshletCount, Constant::k_defaultDispatchMeshThreadGroupCountY, Constant::k_defaultDispatchMeshThreadGroupCountZ);

	return true;
}