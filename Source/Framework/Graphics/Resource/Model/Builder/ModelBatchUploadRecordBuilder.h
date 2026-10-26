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

            auto& l_lodCoveredLayerCountBuffer = a_modelMeshRuntimeData.m_lodCoveredLayerCountBuffer;
            auto& l_outlineNormalBuffer        = a_modelMeshRuntimeData.m_outlineNormalBuffer;

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

            // 頂点ごとの覆われている層の数のストラクチャードバッファーの作成(LODで覆われている頂点を沈めるために、メッシュシェーダーが読む)
            FWK_ASSERT_RETURN_VALUE_IF(!l_lodCoveredLayerCountBuffer.Create(l_modelMeshletData.m_lodCoveredLayerCountList,
                                                                            a_device,
                                                                            a_gpuMemoryAllocator,
                                                                            a_bufferUploadCommandList,
                                                                            a_cbvSRVUAVDescriptorPool),
                                                                            "LODCoveredLayerCountBuffer用StructuredBufferの作成に失敗しました。",
                                                                            false);

            // 頂点ごとの輪郭線を押し出す向き(スムーズ法線)のストラクチャードバッファーの作成
            // ファイルには保存せず、読み込むたびに頂点と三角形から作る(輪郭線を描くメッシュシェーダーが読む)
            const auto& l_outlineNormalList = m_outlineNormalBuilder.BuildOutlineNormalList(a_modelMesh);

            FWK_ASSERT_RETURN_VALUE_IF(!l_outlineNormalBuffer.Create(l_outlineNormalList,
                                                                     a_device,
                                                                     a_gpuMemoryAllocator,
                                                                     a_bufferUploadCommandList,
                                                                     a_cbvSRVUAVDescriptorPool),
                                                                     "OutlineNormalBuffer用StructuredBufferの作成に失敗しました。",
                                                                     false);

            return true;
        }

        void ReleaseModelMeshRuntimeData(Struct::ModelMeshRuntimeDataBase& a_modelMeshRuntimeData) const;

    private:

        ModelOutlineNormalBuilder m_outlineNormalBuilder = {};
    };
}