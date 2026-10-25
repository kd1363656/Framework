#pragma once

namespace FWK::Graphics
{
    class SkeletalAnimationModelBatchUploadRecordBuilder final
    {
    public:

         SkeletalAnimationModelBatchUploadRecordBuilder() = default;
        ~SkeletalAnimationModelBatchUploadRecordBuilder() = default;

        bool CreateSkeletalAnimationModelBatchUploadRecord(const Device&                                                   a_device,
                                                           const GPUMemoryAllocator&                                       a_gpuMemoryAllocator,
                                                                 std::vector<StaticStructuredBuffer::BufferUploadCommand>& a_bufferUploadCommandList,
                                                                 TypeAlias::CBVSRVUAVDescriptorPool&                       a_cbvSRVUAVDescriptorPool,
                                                                 SkeletalAnimationModelRecord&                             a_skeletalAnimationModelRecord) const;

    private:

        // モーフを持つMeshだけ、モーフ用の3つのBuffer(頂点ごとの範囲、移動量、使うチャンネル番号)を作成する
        bool CreateModelMorphBuffer(const Device&                                                   a_device,
                                    const GPUMemoryAllocator&                                       a_gpuMemoryAllocator,
                                    const SkeletalAnimationModelRecord::ModelMesh&                  a_modelMesh,
                                          std::vector<StaticStructuredBuffer::BufferUploadCommand>& a_bufferUploadCommandList,
                                          TypeAlias::CBVSRVUAVDescriptorPool&                       a_cbvSRVUAVDescriptorPool,
                                          SkeletalAnimationModelRecord::ModelMeshRuntimeData&       a_modelMeshRuntimeData) const;

        // 移動量を、GPUで読みやすい形(halfに詰め、ターゲット番号をチャンネル番号へ変えた物)へ変換する
        std::vector<Struct::SBModelMorphVertexDelta> CreateGPUMorphVertexDeltaList(const Struct::ModelMorphData& a_modelMorphData) const;

        // floatを2つhalf(16bitの浮動小数点数)にして、1つのuintへ詰める(下位16bitが1つ目、上位16bitが2つ目)
        std::uint32_t PackHalf2(const float a_firstValue, const float a_secondValue) const;

        void ReleaseCreatedSkeletalAnimationModelStructuredBuffer(std::vector<SkeletalAnimationModelRecord::ModelMesh>& a_modelMeshList) const;

        static constexpr std::size_t k_initialKeyFrameIndex        = 0ULL;
        static constexpr std::size_t k_previousKeyFrameIndexOffset = 1ULL;
        static constexpr std::size_t k_initialTotalKeyFrameCount   = 0ULL;

        static constexpr std::uint32_t k_childHierarchyDepthOffset = 1U;
        static constexpr std::uint32_t k_secondHalfShiftBitCount   = 16U;

        ModelBatchUploadRecordBuilder m_batchUploadRecordBuilder = {};
    };
}