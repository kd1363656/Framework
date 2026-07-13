#pragma once

namespace FWK::Graphics
{
	class ModelBatchUploadRecordBuilder final
	{
	public:

		 ModelBatchUploadRecordBuilder() = default;
		~ModelBatchUploadRecordBuilder() = default;

		template <typename ModelMeshType>
		bool CreateModelMeshBatchUploadRecord(const Device&                                                   a_device,
			                                  const GPUMemoryAllocator&                                       a_gpuMemoryAllocator,
											  const ModelMeshType&                                            a_modelMesh,
			                                        std::vector<StaticStructuredBuffer::BufferUploadCommand>& a_bufferUploadCommandList,
			                                        TypeAlias::CBVSRVUAVDescriptorPool&                       a_cbvSRVUAVDescriptorPool,
			                                        Struct::ModelMeshRuntimeDataBase&                         a_modelMeshRuntimeData) const
		{
			const auto& l_modelMeshletData     = a_modelMesh.m_modelMeshletData;
			      
			auto& l_vertexBuffer            = a_modelMeshRuntimeData.m_vertexBuffer;
			auto& l_meshletBuffer           = a_modelMeshRuntimeData.m_meshletBuffer;
			auto& l_uniqueVertexIndexBuffer = a_modelMeshRuntimeData.m_uniqueVertexIndexBuffer;
			auto& l_primitiveIndexBuffer    = a_modelMeshRuntimeData.m_primitiveIndexBuffer;
			auto& l_meshletBoundsBuffer     = a_modelMeshRuntimeData.m_meshletBoundsBuffer;

			// 頂点バッファーストラクチャードバッファーの作成
			FWK_ASSERT_RETURN_VALUE_IF(!l_vertexBuffer.Create(a_modelMesh.m_modelVertexList,
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

		void ReleaseModelMeshRuntimeData(Struct::ModelMeshRuntimeDataBase& a_modelMeshRuntimeData) const;
	};
}