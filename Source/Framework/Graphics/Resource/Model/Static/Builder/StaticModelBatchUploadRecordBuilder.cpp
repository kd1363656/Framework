#include "StaticModelBatchUploadRecordBuilder.h"

bool FWK::Graphics::StaticModelBatchUploadRecordBuilder::CreateStaticModelBatchUploadRecord(const Device&									                 a_device, 
																							const GPUMemoryAllocator&						                 a_gpuMemoryAllocator, 
																							       std::vector<StaticStructuredBuffer::BufferUploadCommand>& a_bufferUploadCommandList,
																								   TypeAlias::CBVSRVUAVDescriptorPool&	                     a_cbvSRVUAVDescriptorPool,
																								   Graphics::StaticModelRecord&				                 a_staticModelRecord) const
{
	auto& l_modelMeshList = a_staticModelRecord.GetMutableREFModelData().m_modelMeshList;

	FWK_ASSERT_RETURN_VALUE_IF(l_modelMeshList.empty(), "ModelMeshListが空のため、StaticModelBatchUploadの作成に失敗しました。", false);

	for (auto& l_modelMesh : l_modelMeshList)
	{
		// モデルに必要なバッファーをアップロードヒープにデータを渡しデフォルトヒープを作成し、コピー可能な状態にする
		if (!CreateModelBatchUploadRecord(a_device,
										  a_gpuMemoryAllocator,
										  a_bufferUploadCommandList,
										  a_cbvSRVUAVDescriptorPool,
										  l_modelMesh))
		{
			ReleaseCreatedStaticModelStructuredBufferSRV(l_modelMeshList, a_cbvSRVUAVDescriptorPool);

			FWK_ASSERT_RETURN_VALUE("ModelMesh用BatchUploadRecordの作成に失敗したため、StaticModelBatchUploadRecordの作成に失敗しました。", false);
		}
	}

	return true;
}

bool FWK::Graphics::StaticModelBatchUploadRecordBuilder::CreateModelBatchUploadRecord(const Device&				                                      a_device, 
																					  const GPUMemoryAllocator&                                       a_gpuMemoryAllocator,
																							std::vector<StaticStructuredBuffer::BufferUploadCommand>& a_bufferUploadCommandList,
																							TypeAlias::CBVSRVUAVDescriptorPool&		                  a_cbvSRVUAVDescriptorPool,
																							Struct::StaticModelMesh&				                  a_staticModelMesh) const
{
	const auto& l_modelMeshletData     = a_staticModelMesh.m_modelMeshletData;
		  auto& l_modelMeshRuntimeData = a_staticModelMesh.m_modelMeshRuntimeData;

	auto& l_vertexBuffer            = l_modelMeshRuntimeData.m_vertexBuffer;
	auto& l_meshletBuffer           = l_modelMeshRuntimeData.m_meshletBuffer;
	auto& l_uniqueVertexIndexBuffer = l_modelMeshRuntimeData.m_uniqueVertexIndexBuffer;
	auto& l_primitiveIndexBuffer    = l_modelMeshRuntimeData.m_primitiveIndexBuffer;
	auto& l_meshletBoundsBuffer     = l_modelMeshRuntimeData.m_meshletBoundsBuffer;

	// 頂点バッファーストラクチャードバッファーの作成
	FWK_ASSERT_RETURN_VALUE_IF(!l_vertexBuffer.Create(a_staticModelMesh.m_modelVertexList,
													  a_device,
													  a_gpuMemoryAllocator,
													  a_bufferUploadCommandList,
													  a_cbvSRVUAVDescriptorPool),
													  "ModelVertexBuffer用StructuredBufferの作成に失敗しました。",
													  false);

	// メッシュレットストラクチャードバッファーの作成
	FWK_ASSERT_RETURN_VALUE_IF(!l_meshletBuffer.Create(l_modelMeshletData.m_meshletList,
													   a_device,
													   a_gpuMemoryAllocator,
													   a_bufferUploadCommandList,
													   a_cbvSRVUAVDescriptorPool),
													   "MeshletBuffer用StructuredBufferの作成に失敗しました。",
													   false);


	// ユニーク頂点インデックスストラクチャードバッファーの作成
	FWK_ASSERT_RETURN_VALUE_IF(!l_uniqueVertexIndexBuffer.Create(l_modelMeshletData.m_uniqueVertexIndexList,
																 a_device,
																 a_gpuMemoryAllocator,
																 a_bufferUploadCommandList,
																 a_cbvSRVUAVDescriptorPool),
																 "UniqueVertexIndexBuffer用StructuredBufferの作成に失敗しました。",
																 false);

	// プリミティブインデックスバッファストラクチャードバッファーの作成
	FWK_ASSERT_RETURN_VALUE_IF(!l_primitiveIndexBuffer.Create(l_modelMeshletData.m_primitiveIndexList,
															  a_device,
															  a_gpuMemoryAllocator,
															  a_bufferUploadCommandList,
															  a_cbvSRVUAVDescriptorPool),
															  "PrimitiveIndexBuffer用StructuredBufferの作成に失敗しました。",
															  false);

	// メッシュレットカリングストラクチャードバッファーの作成
	FWK_ASSERT_RETURN_VALUE_IF(!l_meshletBoundsBuffer.Create(l_modelMeshletData.m_meshletBoundsList,
															 a_device,
															 a_gpuMemoryAllocator,
															 a_bufferUploadCommandList,
															 a_cbvSRVUAVDescriptorPool),
															 "MeshletBoundsBuffer用StructuredBufferの作成に失敗しました。",
															 false);

	return true;
}

void FWK::Graphics::StaticModelBatchUploadRecordBuilder::ReleaseCreatedStaticModelStructuredBufferSRV(std::vector<Struct::StaticModelMesh>&a_staticModelMeshList, TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool) const
{
	// StructuredBuffer全てのSRVを解放する
	for (auto& l_modelMesh : a_staticModelMeshList)
	{
		auto& l_modelMeshRuntimeData = l_modelMesh.m_modelMeshRuntimeData;

		auto& l_vertexBuffer            = l_modelMeshRuntimeData.m_vertexBuffer;
		auto& l_meshletBuffer           = l_modelMeshRuntimeData.m_meshletBuffer;
		auto& l_uniqueVertexIndexBuffer = l_modelMeshRuntimeData.m_uniqueVertexIndexBuffer;
		auto& l_primitiveIndexBuffer    = l_modelMeshRuntimeData.m_primitiveIndexBuffer;
		auto& l_meshletBoundsBuffer     = l_modelMeshRuntimeData.m_meshletBoundsBuffer;

		l_vertexBuffer.ReleaseImmediatelySRVDescriptorIndex           (a_cbvSRVUAVDescriptorPool);
		l_meshletBuffer.ReleaseImmediatelySRVDescriptorIndex          (a_cbvSRVUAVDescriptorPool);
		l_uniqueVertexIndexBuffer.ReleaseImmediatelySRVDescriptorIndex(a_cbvSRVUAVDescriptorPool);
		l_primitiveIndexBuffer.ReleaseImmediatelySRVDescriptorIndex   (a_cbvSRVUAVDescriptorPool);
		l_meshletBoundsBuffer.ReleaseImmediatelySRVDescriptorIndex    (a_cbvSRVUAVDescriptorPool);
	}
}