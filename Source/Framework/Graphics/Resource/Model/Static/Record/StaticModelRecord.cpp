#include "StaticModelRecord.h"

bool FWK::Graphics::StaticModelRecord::ReserveRelease(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext)
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_retiredFenceValue == Constant::k_unusedFenceValue, "FenceValueが無効のため、StaticModelRecordの遅延解放Queue登録に失敗しました。", false);

	for (auto& l_modelMesh : m_modelData.m_modelMeshList)
	{
		auto& l_modelMeshRuntimeData = l_modelMesh.m_modelMeshRuntimeData;

		Struct::StructuredBufferResourceReleaseRecord l_vertexBuffer            = { a_retiredFenceValue, std::move(l_modelMeshRuntimeData.m_vertexBuffer) };
		Struct::StructuredBufferResourceReleaseRecord l_meshletBuffer           = { a_retiredFenceValue, std::move(l_modelMeshRuntimeData.m_meshletBuffer) };
		Struct::StructuredBufferResourceReleaseRecord l_uniqueVertexIndexBuffer = { a_retiredFenceValue, std::move(l_modelMeshRuntimeData.m_uniqueVertexIndexBuffer) };
		Struct::StructuredBufferResourceReleaseRecord l_primitiveIndexBuffer    = { a_retiredFenceValue, std::move(l_modelMeshRuntimeData.m_primitiveIndexBuffer) };
		Struct::StructuredBufferResourceReleaseRecord l_meshletBoundsBuffer     = { a_retiredFenceValue, std::move(l_modelMeshRuntimeData.m_meshletBoundsBuffer) };

		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!a_resourceReleaseContext.ReserveDeferredReleaseStructuredBufferResourceRecord(l_vertexBuffer),			 "StaticModelRecordのVertexBufferを遅延解放Queueへ登録できませんでした。",            false);
		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!a_resourceReleaseContext.ReserveDeferredReleaseStructuredBufferResourceRecord(l_meshletBuffer),           "StaticModelRecordのMeshletBufferを遅延解放Queueへ登録できませんでした。",           false);
		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!a_resourceReleaseContext.ReserveDeferredReleaseStructuredBufferResourceRecord(l_uniqueVertexIndexBuffer), "StaticModelRecordのUniqueVertexIndexBufferを遅延解放Queueへ登録できませんでした。", false);
		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!a_resourceReleaseContext.ReserveDeferredReleaseStructuredBufferResourceRecord(l_primitiveIndexBuffer),    "StaticModelRecordのPrimitiveIndexBufferを遅延解放Queueへ登録できませんでした。",    false);
		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!a_resourceReleaseContext.ReserveDeferredReleaseStructuredBufferResourceRecord(l_meshletBoundsBuffer),     "StaticModelRecordのMeshletBoundsBufferを遅延解放Queueへ登録できませんでした。",     false);
	}

	return true;
}