#include "SkeletalAnimationModelBatchUploadRecordBuilder.h"

bool FWK::Graphics::SkeletalAnimationModelBatchUploadRecordBuilder::CreateSkeletalAnimationModelBatchUploadRecord(const Device&                                                   a_device,
                                                                                                                  const GPUMemoryAllocator&                                       a_gpuMemoryAllocator,
                                                                                                                        std::vector<StaticStructuredBuffer::BufferUploadCommand>& a_bufferUploadCommandList,
                                                                                                                        TypeAlias::CBVSRVUAVDescriptorPool&                       a_cbvSRVUAVDescriptorPool,
                                                                                                                        SkeletalAnimationModelRecord&                             a_skeletalAnimationModelRecord) const
{
    auto& l_modelMeshList = a_skeletalAnimationModelRecord.GetMutableREFModelData().m_modelMeshList;

    FWK_ASSERT_RETURN_VALUE_IF(l_modelMeshList.empty(), "ModelMeshListが空のため、SkeletalAnimationModelBatchUploadの作成に失敗しました。", false);

    for (auto& l_modelMesh : l_modelMeshList)
    {
        if (l_modelMesh.m_bonePaletteList.empty())
        {
            // 前のModelMeshまでに作成済みのBufferを解放する
            ReleaseCreatedSkeletalAnimationModelStructuredBuffer(l_modelMeshList);

            FWK_ASSERT_RETURN_VALUE("BonePaletteListが空のため、SkeletalAnimationModelBatchUploadの作成に失敗しました。", false);
        }

        // 共通5BufferとBonePaletteBufferのすべてが完成するまで、
        // ModelMesh本体のRuntimeDataへは反映しない
        SkeletalAnimationModelRecord::ModelMeshRuntimeData l_modelMeshRuntimeData = {};

        // Vertex、Meshlet、MeshletBoundsの共通Bufferを作成する
        if (!m_batchUploadRecordBuilder.CreateModelMeshBatchUploadRecord(a_device,
                                                                         a_gpuMemoryAllocator,
                                                                         l_modelMesh,
                                                                         a_bufferUploadCommandList,
                                                                         a_cbvSRVUAVDescriptorPool,
                                                                         l_modelMeshRuntimeData))
        {
            // 現在作成中のl_modelMeshRuntimeDataは、
            // この関数を抜ける際に自動的にReleaseされる。
            // ここでは前のModelMeshまでに完成しているBufferを解放する
            ReleaseCreatedSkeletalAnimationModelStructuredBuffer(l_modelMeshList);

            FWK_ASSERT_RETURN_VALUE("SkeletalAnimationModelの共通MeshBuffer作成に失敗しました。", false);
        }

        // SkeletalAnimationModelにのみ存在するBonePaletteBufferを作成する
        if (!l_modelMeshRuntimeData.m_bonePaletteBuffer.Create(l_modelMesh.m_bonePaletteList,
                                                               a_device,
                                                               a_gpuMemoryAllocator,
                                                               a_bufferUploadCommandList,
                                                               a_cbvSRVUAVDescriptorPool))
        {
            // l_modelMeshRuntimeData内の共通Bufferは、
            // 関数を抜ける際に自動的にReleaseされる
            ReleaseCreatedSkeletalAnimationModelStructuredBuffer(l_modelMeshList);

            FWK_ASSERT_RETURN_VALUE("BonePaletteBuffer用StructuredBufferの作成に失敗しました。", false);
        }

        // モーフを持つMeshだけ、モーフ用のBufferを作成する
        if (!CreateModelMorphBuffer(a_device,
                                    a_gpuMemoryAllocator,
                                    l_modelMesh,
                                    a_bufferUploadCommandList,
                                    a_cbvSRVUAVDescriptorPool,
                                    l_modelMeshRuntimeData))
        {
            ReleaseCreatedSkeletalAnimationModelStructuredBuffer(l_modelMeshList);

            FWK_ASSERT_RETURN_VALUE("モーフ用StructuredBufferの作成に失敗しました。", false);
        }

        // 共通5BufferとBonePaletteBufferがすべて完成したら、
        // 派生RuntimeData全体をModelMeshへ移動する
        l_modelMesh.m_modelMeshRuntimeData = std::move(l_modelMeshRuntimeData);
    }

    return true;
}
bool FWK::Graphics::SkeletalAnimationModelBatchUploadRecordBuilder::CreateModelMorphBuffer(const Device&                                                   a_device,
                                                                                           const GPUMemoryAllocator&                                       a_gpuMemoryAllocator,
                                                                                           const SkeletalAnimationModelRecord::ModelMesh&                  a_modelMesh,
                                                                                                 std::vector<StaticStructuredBuffer::BufferUploadCommand>& a_bufferUploadCommandList,
                                                                                                 TypeAlias::CBVSRVUAVDescriptorPool&                       a_cbvSRVUAVDescriptorPool,
                                                                                                 SkeletalAnimationModelRecord::ModelMeshRuntimeData&       a_modelMeshRuntimeData) const
{
    const auto& l_modelMorphData = a_modelMesh.m_modelMorphData;

    // モーフを持たないMeshはBufferを作成しない(SRVの番号は無効値のままなので、シェーダーはモーフを行わない)
    if (l_modelMorphData.m_targetList.empty()) { return true; }

    FWK_ASSERT_RETURN_VALUE_IF(l_modelMorphData.m_targetChannelIndexList.size() != l_modelMorphData.m_targetList.size(), "モーフのチャンネル番号が決まっていないため、モーフ用Bufferを作成できません。", false);

    const auto& l_gpuMorphVertexDeltaList = CreateGPUMorphVertexDeltaList(l_modelMorphData);

    FWK_ASSERT_RETURN_VALUE_IF(!a_modelMeshRuntimeData.m_morphVertexRangeBuffer.Create(l_modelMorphData.m_vertexRangeList,
                                                                                        a_device,
                                                                                        a_gpuMemoryAllocator,
                                                                                        a_bufferUploadCommandList,
                                                                                        a_cbvSRVUAVDescriptorPool),
                                                                                        "MorphVertexRangeBufferの作成に失敗しました。",
                                                                                        false);

    FWK_ASSERT_RETURN_VALUE_IF(!a_modelMeshRuntimeData.m_morphDeltaBuffer.Create(l_gpuMorphVertexDeltaList,
                                                                                  a_device,
                                                                                  a_gpuMemoryAllocator,
                                                                                  a_bufferUploadCommandList,
                                                                                  a_cbvSRVUAVDescriptorPool),
                                                                                  "MorphDeltaBufferの作成に失敗しました。",
                                                                                  false);

    FWK_ASSERT_RETURN_VALUE_IF(!a_modelMeshRuntimeData.m_morphChannelListBuffer.Create(l_modelMorphData.m_targetChannelIndexList,
                                                                                        a_device,
                                                                                        a_gpuMemoryAllocator,
                                                                                        a_bufferUploadCommandList,
                                                                                        a_cbvSRVUAVDescriptorPool),
                                                                                        "MorphChannelListBufferの作成に失敗しました。",
                                                                                        false);

    // LODを選ぶときに、効かせているターゲットの崩れだけを足すために使う
    FWK_ASSERT_RETURN_VALUE_IF(!a_modelMeshRuntimeData.m_morphLODTargetErrorBuffer.Create(l_modelMorphData.m_lodTargetMorphErrorList,
                                                                                           a_device,
                                                                                           a_gpuMemoryAllocator,
                                                                                           a_bufferUploadCommandList,
                                                                                           a_cbvSRVUAVDescriptorPool),
                                                                                           "MorphLODTargetErrorBufferの作成に失敗しました。",
                                                                                           false);

    return true;
}

std::vector<FWK::Struct::SBModelMorphVertexDelta> FWK::Graphics::SkeletalAnimationModelBatchUploadRecordBuilder::CreateGPUMorphVertexDeltaList(const Struct::ModelMorphData& a_modelMorphData) const
{
    const auto& l_vertexDeltaList       = a_modelMorphData.m_vertexDeltaList;
    const auto& l_targetChannelIndexList = a_modelMorphData.m_targetChannelIndexList;

    std::vector<Struct::SBModelMorphVertexDelta> l_gpuMorphVertexDeltaList = {};

    l_gpuMorphVertexDeltaList.reserve(l_vertexDeltaList.size());

    for (const auto& l_vertexDelta : l_vertexDeltaList)
    {
        const auto& l_deltaPosition = l_vertexDelta.m_deltaPosition;
        const auto& l_deltaNormal   = l_vertexDelta.m_deltaNormal;

        Struct::SBModelMorphVertexDelta l_gpuMorphVertexDelta = {};

        l_gpuMorphVertexDelta.m_deltaPositionXY       = PackHalf2(l_deltaPosition.x, l_deltaPosition.y);
        l_gpuMorphVertexDelta.m_deltaPositionZNormalX = PackHalf2(l_deltaPosition.z, l_deltaNormal.x);
        l_gpuMorphVertexDelta.m_deltaNormalYZ         = PackHalf2(l_deltaNormal.y,   l_deltaNormal.z);

        // Meshの中のターゲット番号を、モデル全体のチャンネル番号へ変える(ウェイトはチャンネルごとに持つため)
        l_gpuMorphVertexDelta.m_channelIndex = l_targetChannelIndexList[l_vertexDelta.m_targetIndex];

        l_gpuMorphVertexDeltaList.emplace_back(l_gpuMorphVertexDelta);
    }

    return l_gpuMorphVertexDeltaList;
}

std::uint32_t FWK::Graphics::SkeletalAnimationModelBatchUploadRecordBuilder::PackHalf2(const float a_firstValue, const float a_secondValue) const
{
    // XMConvertFloatToHalf : float(32bit)をhalf(16bit)へ変換する(DirectXMathの関数)
    const std::uint32_t l_firstHalf  = DirectX::PackedVector::XMConvertFloatToHalf(a_firstValue);
    const std::uint32_t l_secondHalf = DirectX::PackedVector::XMConvertFloatToHalf(a_secondValue);

    return l_firstHalf | (l_secondHalf << k_secondHalfShiftBitCount);
}

void FWK::Graphics::SkeletalAnimationModelBatchUploadRecordBuilder::ReleaseCreatedSkeletalAnimationModelStructuredBuffer(std::vector<SkeletalAnimationModelRecord::ModelMesh>& a_modelMeshList) const
{
    for (auto& l_modelMesh : a_modelMeshList)
    {
        auto& l_modelMeshRuntimeData = l_modelMesh.m_modelMeshRuntimeData;

        // StaticとSkeletalで共通する5種類のBufferを解放する
        m_batchUploadRecordBuilder.ReleaseModelMeshRuntimeData(l_modelMeshRuntimeData);

        // SkeletalAnimationModel固有のBonePaletteBufferとモーフ用Bufferを解放する
        l_modelMeshRuntimeData.m_bonePaletteBuffer.Release        ();
        l_modelMeshRuntimeData.m_morphVertexRangeBuffer.Release   ();
        l_modelMeshRuntimeData.m_morphDeltaBuffer.Release         ();
        l_modelMeshRuntimeData.m_morphChannelListBuffer.Release   ();
        l_modelMeshRuntimeData.m_morphLODTargetErrorBuffer.Release();
    }
}