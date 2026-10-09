#include "SkeletalAnimationModelBatchUploadRecordBuilder.h"

bool FWK::Graphics::SkeletalAnimationModelBatchUploadRecordBuilder::CreateSkeletalAnimationModelBatchUploadRecord(const Device&                                   a_device,
                                                                                                                  const GPUMemoryAllocator&                       a_gpuMemoryAllocator,
                                                                                                                        std::vector<Struct::BufferUploadCommand>& a_bufferUploadCommandList,
                                                                                                                        TypeAlias::CBVSRVUAVDescriptorPool&       a_cbvSRVUAVDescriptorPool,
                                                                                                                        SkeletalAnimationModelRecord&             a_skeletalAnimationModelRecord) const
{
    auto& l_meshList = a_skeletalAnimationModelRecord.GetMutableREFModelData().m_meshList;

    FWK_ASSERT_RETURN_VALUE_IF(l_meshList.empty(), "ModelMeshListが空のため、SkeletalAnimationModelBatchUploadの作成に失敗しました。", false);

    for (auto& l_modelMesh : l_meshList)
    {
        if (l_modelMesh.m_bonePaletteList.empty())
        {
            // 前のModelMeshまでに作成済みのBufferを解放する
            ReleaseCreatedSkeletalAnimationModelStructuredBuffer(l_meshList);

            FWK_ASSERT_RETURN_VALUE("BonePaletteListが空のため、SkeletalAnimationModelBatchUploadの作成に失敗しました。", false);
        }

        // 共通5BufferとBonePaletteBufferのすべてが完成するまで、
        // ModelMesh本体のRuntimeDataへは反映しない
        Struct::SkeletalAnimationModelMeshRuntimeData l_meshRuntimeData = {};

        // Vertex、Meshlet、MeshletBoundsの共通Bufferを作成する
        if (!m_batchUploadRecordBuilder.CreateModelMeshBatchUploadRecord(a_device,
                                                                         a_gpuMemoryAllocator,
                                                                         l_modelMesh,
                                                                         a_bufferUploadCommandList,
                                                                         a_cbvSRVUAVDescriptorPool,
                                                                         l_meshRuntimeData))
        {
            // 現在作成中のl_meshRuntimeDataは、
            // この関数を抜ける際に自動的にReleaseされる
            // ここでは前のModelMeshまでに完成しているBufferを解放する
            ReleaseCreatedSkeletalAnimationModelStructuredBuffer(l_meshList);

            FWK_ASSERT_RETURN_VALUE("SkeletalAnimationModelの共通MeshBuffer作成に失敗しました。", false);
        }

        // SkeletalAnimationModelにのみ存在するBonePaletteBufferを作成する
        if (!l_meshRuntimeData.m_bonePaletteBuffer.Create(l_modelMesh.m_bonePaletteList,
                                                               a_device,
                                                               a_gpuMemoryAllocator,
                                                               a_bufferUploadCommandList,
                                                               a_cbvSRVUAVDescriptorPool))
        {
            // l_meshRuntimeData内の共通Bufferは、
            // 関数を抜ける際に自動的にReleaseされる
            ReleaseCreatedSkeletalAnimationModelStructuredBuffer(l_meshList);

            FWK_ASSERT_RETURN_VALUE("BonePaletteBuffer用StructuredBufferの作成に失敗しました。", false);
        }

        // 共通5BufferとBonePaletteBufferがすべて完成したら、
        // 派生RuntimeData全体をModelMeshへ移動する
        l_modelMesh.m_meshRuntimeData = std::move(l_meshRuntimeData);
    }

    return true;
}
void FWK::Graphics::SkeletalAnimationModelBatchUploadRecordBuilder::ReleaseCreatedSkeletalAnimationModelStructuredBuffer(std::vector<Struct::SkeletalAnimationModelMesh>& a_modelMeshList) const
{
    for (auto& l_modelMesh : a_modelMeshList)
    {
        auto& l_meshRuntimeData = l_modelMesh.m_meshRuntimeData;

        // StaticとSkeletalで共通する5種類のBufferを解放する
        m_batchUploadRecordBuilder.ReleaseModelMeshRuntimeData(l_meshRuntimeData);

        // SkeletalAnimationModel固有のBonePaletteBufferを解放する
        l_meshRuntimeData.m_bonePaletteBuffer.Release();
    }
}