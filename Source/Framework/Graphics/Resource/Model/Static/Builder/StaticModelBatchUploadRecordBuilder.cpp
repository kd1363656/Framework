#include "StaticModelBatchUploadRecordBuilder.h"

bool FWK::Graphics::StaticModelBatchUploadRecordBuilder::CreateStaticModelBatchUploadRecord(const Device&									 a_device, 
																							const GPUMemoryAllocator&						 a_gpuMemoryAllocator, 
																							       std::vector<Struct::BufferUploadCommand>& a_bufferUploadCommandList,
																								   TypeAlias::SRVDescriptorPool&			 a_srvDescriptorPool,
																								   Graphics::StaticModelRecord&				 a_staticModelRecord) const
{
	auto& l_modelMeshList = a_staticModelRecord.GetMutableREFModelData().m_modelMeshList;

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_modelMeshList.empty(), "ModelMeshListが空のため、StaticModelBatchUploadの作成に失敗しました。", false);

	for (auto& l_modelMesh : l_modelMeshList)
	{
		if (!CreateModelBatchUploadRecord(a_device,
										  a_gpuMemoryAllocator,
										  a_bufferUploadCommandList,
										  a_srvDescriptorPool,
										  l_modelMesh))
		{
			ReleaseCreatedStaticModelStructuredBufferSRV(l_modelMeshList, a_srvDescriptorPool);
			FWK_ASSERT_RETURN_VALUE					    ("ModelMesh用BatchUploadRecordの作成に失敗したため、StaticModelBatchUploadRecordの作成に失敗しました。", false);
		}
	}

	return true;
}

bool FWK::Graphics::StaticModelBatchUploadRecordBuilder::CreateModelBatchUploadRecord(const Device&				                      a_device, 
																					  const GPUMemoryAllocator&                       a_gpuMemoryAllocator,
																							std::vector<Struct::BufferUploadCommand>& a_bufferUploadCommandList,
																							TypeAlias::SRVDescriptorPool&			  a_srvDescriptorPool,
																							Struct::StaticModelMesh&				  a_staticModelMesh) const
{
	const auto& l_modelMeshletData     = a_staticModelMesh.m_modelMeshletData;
		  auto& l_modelMeshRuntimeData = a_staticModelMesh.m_modelMeshRuntimeData;

		// 頂点バッファー用アップロードバッファの作成
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!CreateBufferUploadCommand(a_staticModelMesh.m_modelVertexList,
																 a_device,
																 a_gpuMemoryAllocator,
																 a_bufferUploadCommandList,
																 l_modelMeshRuntimeData.m_vertexBuffer.m_bufferGPUResource),
																 "ModelVertexBuffer用BufferUploadCommandの作成に失敗しました。",
																 false);

	// メッシュレットバッファー用アップロードバッファの作成
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!CreateBufferUploadCommand(l_modelMeshletData.m_meshletList,
																 a_device,
																 a_gpuMemoryAllocator,
																 a_bufferUploadCommandList,
																 l_modelMeshRuntimeData.m_meshletBuffer.m_bufferGPUResource),
																 "MeshletBuffer用BufferUploadCommandの作成に失敗しました。",
																 false);


	// ユニーク頂点インデックスバッファー用アップロードバッファの作成
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!CreateBufferUploadCommand(l_modelMeshletData.m_uniqueVertexIndexList,
																 a_device,
																 a_gpuMemoryAllocator,
																 a_bufferUploadCommandList,
																 l_modelMeshRuntimeData.m_uniqueVertexIndexBuffer.m_bufferGPUResource),
																 "UniqueVertexIndexBuffer用BufferUploadCommandの作成に失敗しました。",
																 false);

	// プリミティブインデックスバッファー用アップロードバッファの作成
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!CreateBufferUploadCommand(l_modelMeshletData.m_primitiveIndexList,
																 a_device,
																 a_gpuMemoryAllocator,
																 a_bufferUploadCommandList,
																 l_modelMeshRuntimeData.m_primitiveIndexBuffer.m_bufferGPUResource),
																 "PrimitiveIndexBuffer用BufferUploadCommandの作成に失敗しました。",
																 false);

	// メッシュレットカリングバッファー用アップロードバッファの作成
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!CreateBufferUploadCommand(l_modelMeshletData.m_meshletBoundsList,
																 a_device,
																 a_gpuMemoryAllocator,
																 a_bufferUploadCommandList,
																 l_modelMeshRuntimeData.m_meshletBoundsBuffer.m_bufferGPUResource),
																 "MeshletBoundsBuffer用BufferUploadCommandの作成に失敗しました。",
																 false);

	
	// 頂点バッファー用SRVの作成
	auto& l_vertexBuffer = l_modelMeshRuntimeData.m_vertexBuffer;

	CreateStructuredBufferResource(a_staticModelMesh.m_modelVertexList,
								   a_device,
								   a_srvDescriptorPool,
								   l_vertexBuffer);

	// メッシュレットバッファー用SRVの作成
	auto& l_meshletBuffer = l_modelMeshRuntimeData.m_meshletBuffer;

	CreateStructuredBufferResource(l_modelMeshletData.m_meshletList,
								   a_device,
								   a_srvDescriptorPool,
								   l_meshletBuffer);

	// ユニーク頂点インデックスバッファー用SRVの作成
	auto& l_uniqueVertexIndexBuffer = l_modelMeshRuntimeData.m_uniqueVertexIndexBuffer;

	CreateStructuredBufferResource(l_modelMeshletData.m_uniqueVertexIndexList,
								   a_device,
								   a_srvDescriptorPool,
								   l_uniqueVertexIndexBuffer);

	// プリミティブインデックスバッファー用SRVの作成
	auto& l_primitiveIndexBuffer = l_modelMeshRuntimeData.m_primitiveIndexBuffer;

	CreateStructuredBufferResource(l_modelMeshletData.m_primitiveIndexList,
								   a_device,
								   a_srvDescriptorPool,
								   l_primitiveIndexBuffer);

	// メッシュレットカリング用SRVの作成
	auto& l_meshletBoundsBuffer = l_modelMeshRuntimeData.m_meshletBoundsBuffer;

	CreateStructuredBufferResource(l_modelMeshletData.m_meshletBoundsList,
								   a_device,
								   a_srvDescriptorPool,
								   l_meshletBoundsBuffer);

	return true;
}

void FWK::Graphics::StaticModelBatchUploadRecordBuilder::ReleaseCreatedStructuredBufferSRV(Struct::StructuredBufferResource& a_structuredBufferResource, TypeAlias::SRVDescriptorPool& a_srvDescriptorPool) const
{
	if (a_structuredBufferResource.m_srvDescriptorIndex == Constant::k_invalidDescriptorIndex) { return; }

	a_srvDescriptorPool.Release(a_structuredBufferResource.m_srvDescriptorIndex);

	a_structuredBufferResource.m_srvDescriptorIndex = Constant::k_invalidDescriptorIndex;

}
void FWK::Graphics::StaticModelBatchUploadRecordBuilder::ReleaseCreatedStaticModelStructuredBufferSRV(std::vector<Struct::StaticModelMesh>&a_staticModelMeshList, TypeAlias::SRVDescriptorPool & a_srvDescriptorPool) const
{
	// StructuredBuffer全てのSRVを解放する
	for (auto& l_modelMesh : a_staticModelMeshList)
	{
		ReleaseCreatedStructuredBufferSRV(l_modelMesh.m_modelMeshRuntimeData.m_vertexBuffer,		    a_srvDescriptorPool);
		ReleaseCreatedStructuredBufferSRV(l_modelMesh.m_modelMeshRuntimeData.m_meshletBuffer,           a_srvDescriptorPool);
		ReleaseCreatedStructuredBufferSRV(l_modelMesh.m_modelMeshRuntimeData.m_uniqueVertexIndexBuffer, a_srvDescriptorPool);
		ReleaseCreatedStructuredBufferSRV(l_modelMesh.m_modelMeshRuntimeData.m_primitiveIndexBuffer,    a_srvDescriptorPool);
		ReleaseCreatedStructuredBufferSRV(l_modelMesh.m_modelMeshRuntimeData.m_meshletBoundsBuffer,     a_srvDescriptorPool);
	}
}