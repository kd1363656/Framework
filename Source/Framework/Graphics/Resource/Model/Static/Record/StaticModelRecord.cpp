#include "StaticModelRecord.h"

bool FWK::Graphics::StaticModelRecord::ReserveRelease(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext)
{
	FWK_ASSERT_RETURN_VALUE_IF(a_retiredFenceValue == Fence::k_unusedFenceValue, "FenceValueが無効のため、StaticModelRecordの遅延解放Queue登録に失敗しました。", false);

	for (auto& l_modelMesh : m_modelData.m_modelMeshList)
	{
		auto& l_modelMeshRuntimeData = l_modelMesh.m_modelMeshRuntimeData;

		auto& l_vertexBuffer            = l_modelMeshRuntimeData.m_vertexBuffer;
		auto& l_meshletBuffer           = l_modelMeshRuntimeData.m_meshletBuffer;
		auto& l_uniqueVertexIndexBuffer = l_modelMeshRuntimeData.m_uniqueVertexIndexBuffer;
		auto& l_primitiveIndexBuffer    = l_modelMeshRuntimeData.m_primitiveIndexBuffer;
		auto& l_meshletBoundsBuffer     = l_modelMeshRuntimeData.m_meshletBoundsBuffer;

		FWK_ASSERT_RETURN_VALUE_IF(!l_vertexBuffer.ReserveRelease           (a_retiredFenceValue, a_resourceReleaseContext), "StaticModelRecordのVertexBufferを遅延解放Queueへ登録できませんでした。",            false);
		FWK_ASSERT_RETURN_VALUE_IF(!l_meshletBuffer.ReserveRelease          (a_retiredFenceValue, a_resourceReleaseContext), "StaticModelRecordのMeshletBufferを遅延解放Queueへ登録できませんでした。",           false);
		FWK_ASSERT_RETURN_VALUE_IF(!l_uniqueVertexIndexBuffer.ReserveRelease(a_retiredFenceValue, a_resourceReleaseContext), "StaticModelRecordのUniqueVertexIndexBufferを遅延解放Queueへ登録できませんでした。", false);
		FWK_ASSERT_RETURN_VALUE_IF(!l_primitiveIndexBuffer.ReserveRelease   (a_retiredFenceValue, a_resourceReleaseContext), "StaticModelRecordのPrimitiveIndexBufferを遅延解放Queueへ登録できませんでした。",    false);
		FWK_ASSERT_RETURN_VALUE_IF(!l_meshletBoundsBuffer.ReserveRelease    (a_retiredFenceValue, a_resourceReleaseContext), "StaticModelRecordのMeshletBoundsBufferを遅延解放Queueへ登録できませんでした。",     false);
	}

	return true;
}