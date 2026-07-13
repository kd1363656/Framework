#include "StaticModelBatchUploadRecordBuilder.h"

bool FWK::Graphics::StaticModelBatchUploadRecordBuilder::CreateStaticModelBatchUploadRecord(const Device&									                 a_device, 
																							const GPUMemoryAllocator&						                 a_gpuMemoryAllocator, 
																							       std::vector<StaticStructuredBuffer::BufferUploadCommand>& a_bufferUploadCommandList,
																								   TypeAlias::CBVSRVUAVDescriptorPool&	                     a_cbvSRVUAVDescriptorPool,
																								   StaticModelRecord&				                         a_staticModelRecord) const
{
	auto& l_modelMeshList = a_staticModelRecord.GetMutableREFModelData().m_modelMeshList;

	FWK_ASSERT_RETURN_VALUE_IF(l_modelMeshList.empty(), "ModelMeshListが空のため、StaticModelBatchUploadの作成に失敗しました。", false);

	for (auto& l_modelMesh : l_modelMeshList)
	{
		// 共通Bufferがすべて完成するまでは
		// ModelMesh本体のRuntimeDataへ反映しない
		Struct::ModelMeshRuntimeDataBase l_modelMeshRuntimeData = {};

		// モデルに必要なバッファーをアップロードヒープにデータを渡しデフォルトヒープを作成し、コピー可能な状態にする
		if (!m_batchUploadRecordBuilder.CreateModelMeshBatchUploadRecord(a_device,
										                                 a_gpuMemoryAllocator,
																		 l_modelMesh,
										                                 a_bufferUploadCommandList,
										                                 a_cbvSRVUAVDescriptorPool,
										                                 l_modelMeshRuntimeData))
		{
			ReleaseCreatedStaticModelStructuredBuffer(l_modelMeshList);

			FWK_ASSERT_RETURN_VALUE("ModelMesh用BatchUploadRecordの作成に失敗したため、StaticModelBatchUploadRecordの作成に失敗しました。", false);
		}

		// 5個の共通Bufferがすべて完成したため、
		// ModelMeshのRuntimeDataへまとめて移動する
		l_modelMesh.m_modelMeshRuntimeData = std::move(l_modelMeshRuntimeData);
	}

	return true;
}

void FWK::Graphics::StaticModelBatchUploadRecordBuilder::ReleaseCreatedStaticModelStructuredBuffer(std::vector<StaticModelRecord::ModelMesh>& a_modelMeshList) const
{
	for (auto& l_modelMesh : a_modelMeshList)
	{
		m_batchUploadRecordBuilder.ReleaseModelMeshRuntimeData(l_modelMesh.m_modelMeshRuntimeData);
	}
}