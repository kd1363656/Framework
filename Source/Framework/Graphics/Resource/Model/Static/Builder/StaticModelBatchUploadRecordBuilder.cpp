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
																 l_modelMeshRuntimeData.m_vertexBuffer.GetMutableREFBufferGPUResource()),
																 "ModelVertexBuffer用BufferUploadCommandの作成に失敗しました。",
																 false);

	// メッシュレットバッファー用アップロードバッファの作成
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!CreateBufferUploadCommand(l_modelMeshletData.m_meshletList,
																 a_device,
																 a_gpuMemoryAllocator,
																 a_bufferUploadCommandList,
																 l_modelMeshRuntimeData.m_meshletBuffer.GetMutableREFBufferGPUResource()),
																 "MeshletBuffer用BufferUploadCommandの作成に失敗しました。",
																 false);


	// ユニーク頂点インデックスバッファー用アップロードバッファの作成
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!CreateBufferUploadCommand(l_modelMeshletData.m_uniqueVertexIndexList,
																 a_device,
																 a_gpuMemoryAllocator,
																 a_bufferUploadCommandList,
																 l_modelMeshRuntimeData.m_uniqueVertexIndexBuffer.GetMutableREFBufferGPUResource()),
																 "UniqueVertexIndexBuffer用BufferUploadCommandの作成に失敗しました。",
																 false);

	// プリミティブインデックスバッファー用アップロードバッファの作成
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!CreateBufferUploadCommand(l_modelMeshletData.m_primitiveIndexList,
																 a_device,
																 a_gpuMemoryAllocator,
																 a_bufferUploadCommandList,
																 l_modelMeshRuntimeData.m_primitiveIndexBuffer.GetMutableREFBufferGPUResource()),
																 "PrimitiveIndexBuffer用BufferUploadCommandの作成に失敗しました。",
																 false);

	// メッシュレットカリングバッファー用アップロードバッファの作成
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!CreateBufferUploadCommand(l_modelMeshletData.m_meshletBoundsList,
																 a_device,
																 a_gpuMemoryAllocator,
																 a_bufferUploadCommandList,
																 l_modelMeshRuntimeData.m_meshletBoundsBuffer.GetMutableREFBufferGPUResource()),
																 "MeshletBoundsBuffer用BufferUploadCommandの作成に失敗しました。",
																 false);

	
	// 頂点バッファー用SRVの作成
	auto& l_vertexBuffer = l_modelMeshRuntimeData.m_vertexBuffer;

	l_vertexBuffer.CreateStructuredBufferSRV(a_staticModelMesh.m_modelVertexList, a_device, a_srvDescriptorPool);
	
	// メッシュレットバッファー用SRVの作成
	auto& l_meshletBuffer = l_modelMeshRuntimeData.m_meshletBuffer;

	l_meshletBuffer.CreateStructuredBufferSRV(l_modelMeshletData.m_meshletList, a_device, a_srvDescriptorPool);
	
	// ユニーク頂点インデックスバッファー用SRVの作成
	auto& l_uniqueVertexIndexBuffer = l_modelMeshRuntimeData.m_uniqueVertexIndexBuffer;

	l_uniqueVertexIndexBuffer.CreateStructuredBufferSRV(l_modelMeshletData.m_uniqueVertexIndexList, a_device, a_srvDescriptorPool);
	
	// プリミティブインデックスバッファー用SRVの作成
	auto& l_primitiveIndexBuffer = l_modelMeshRuntimeData.m_primitiveIndexBuffer;

	l_primitiveIndexBuffer.CreateStructuredBufferSRV(l_modelMeshletData.m_primitiveIndexList, a_device, a_srvDescriptorPool);

	// メッシュレットカリング用SRVの作成
	auto& l_meshletBoundsBuffer = l_modelMeshRuntimeData.m_meshletBoundsBuffer;

	l_meshletBoundsBuffer.CreateStructuredBufferSRV(l_modelMeshletData.m_meshletBoundsList, a_device, a_srvDescriptorPool);
	
	return true;
}

void FWK::Graphics::StaticModelBatchUploadRecordBuilder::ReleaseCreatedStaticModelStructuredBufferSRV(std::vector<Struct::StaticModelMesh>&a_staticModelMeshList, TypeAlias::SRVDescriptorPool & a_srvDescriptorPool) const
{
	// StructuredBuffer全てのSRVを解放する
	for (const auto& l_modelMesh : a_staticModelMeshList)
	{
		const auto& l_modelMeshRuntimeData = l_modelMesh.m_modelMeshRuntimeData;

		auto& l_vertexBuffer            = l_modelMeshRuntimeData.m_vertexBuffer;
		auto& l_meshletBuffer           = l_modelMeshRuntimeData.m_meshletBuffer;
		auto& l_uniqueVertexIndexBuffer = l_modelMeshRuntimeData.m_uniqueVertexIndexBuffer;
		auto& l_primitiveIndexBuffer    = l_modelMeshRuntimeData.m_primitiveIndexBuffer;
		auto& l_meshletBoundsBuffer     = l_modelMeshRuntimeData.m_meshletBoundsBuffer;

		if (l_vertexBuffer.GetVALSRVDescriptorIndex() != Constant::k_invalidDescriptorIndex)
		{
			a_srvDescriptorPool.Release(l_vertexBuffer.GetVALSRVDescriptorIndex());
		}

		if (l_meshletBuffer.GetVALSRVDescriptorIndex() != Constant::k_invalidDescriptorIndex)
		{
			a_srvDescriptorPool.Release(l_meshletBuffer.GetVALSRVDescriptorIndex());
		}

		if (l_uniqueVertexIndexBuffer.GetVALSRVDescriptorIndex() != Constant::k_invalidDescriptorIndex)
		{
			a_srvDescriptorPool.Release(l_uniqueVertexIndexBuffer.GetVALSRVDescriptorIndex());
		}

		if (l_primitiveIndexBuffer.GetVALSRVDescriptorIndex() != Constant::k_invalidDescriptorIndex)
		{
			a_srvDescriptorPool.Release(l_primitiveIndexBuffer.GetVALSRVDescriptorIndex());
		}

		if (l_meshletBoundsBuffer.GetVALSRVDescriptorIndex() != Constant::k_invalidDescriptorIndex)
		{
			a_srvDescriptorPool.Release(l_meshletBoundsBuffer.GetVALSRVDescriptorIndex());
		}
	}
}