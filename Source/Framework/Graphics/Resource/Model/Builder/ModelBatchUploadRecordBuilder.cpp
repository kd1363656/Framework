#include "ModelBatchUploadRecordBuilder.h"

void FWK::Graphics::ModelBatchUploadRecordBuilder::ReleaseModelMeshRuntimeData(Struct::ModelMeshRuntimeDataBase& a_modelMeshRuntimeData) const
{
	// StructuredBuffer全てのSRVを解放する
	a_modelMeshRuntimeData.m_vertexBuffer.Release           ();
	a_modelMeshRuntimeData.m_meshletBuffer.Release          ();
	a_modelMeshRuntimeData.m_uniqueVertexIndexBuffer.Release();
	a_modelMeshRuntimeData.m_primitiveIndexBuffer.Release   ();
	a_modelMeshRuntimeData.m_meshletBoundsBuffer.Release    ();
}