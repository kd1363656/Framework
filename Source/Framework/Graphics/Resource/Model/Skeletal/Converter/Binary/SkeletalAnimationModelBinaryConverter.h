#pragma once

namespace FWK::Converter
{
    class SkeletalAnimationModelBinaryConverter final : public ModelBinaryConverterBase
    {
    private:

        struct ModelBinaryHeader final
        {
            std::uint64_t m_fileSize    = k_emptyAssetFileSize;
            std::uint16_t m_version     = k_modelAssetVersion;
            std::uint16_t m_assetTypeID = k_modelAssetTypeID;

            std::uint64_t m_modelMeshCount      = Constant::k_emptyModelMeshCount;
            std::uint64_t m_boneCount           = k_emptyBoneCount;
            std::uint64_t m_motionSequenceCount = k_emptyMotionSequenceCount;
        };

        struct ModelBoneBinaryHeader final
        {
            std::uint64_t m_boneNameSize = k_emptyBoneNameSize;

            std::uint32_t m_parentBoneIndex = Constant::k_invalidBoneIndex;
        };

        struct ModelMotionSequenceBinaryHeader final
        {
            float m_durationSecond = Constant::k_initialAnimationDurationSecond;
            float m_frameRate      = Constant::k_defaultAnimationFrameRate;

            std::uint64_t m_motionNameSize = k_emptyMotionNameSize;

            std::uint64_t m_boneMotionTrackCount = k_emptyBoneMotionTrackCount;
        };

        struct ModelBoneMotionTrackBinaryHeader final
        {
            std::uint64_t m_keyFrameCount = k_emptyKeyFrameCount;

            std::uint32_t m_boneIndex = Constant::k_invalidBoneIndex;
        };

    public:

         SkeletalAnimationModelBinaryConverter()          = default;
        ~SkeletalAnimationModelBinaryConverter() override = default;

        bool Load(const std::filesystem::path& a_filePath, Graphics::SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord);

        bool Save(const std::filesystem::path& a_filePath, const Graphics::SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord);

        static constexpr std::uint64_t k_emptyBonePaletteCount     = 0ULL;
        static constexpr std::uint64_t k_emptyBoneCount            = 0ULL;
        static constexpr std::uint64_t k_emptyMotionSequenceCount  = 0ULL;

    private:

        void FailLoad(Struct::SkeletalAnimationModelData& a_modelData);

        ModelBinaryHeader CreateModelBinaryHeader(const Struct::SkeletalAnimationModelData& a_modelData, const std::uint64_t& a_fileSize) const;

        ModelBoneBinaryHeader CreateModelBoneBinaryHeader(const Struct::SkeletalAnimationModelBone& a_modelBone) const;

        ModelMotionSequenceBinaryHeader CreateMotionSequenceBinaryHeader(const Struct::SkeletalAnimationModelMotionSequence& a_modelMotionSequence) const;

        ModelBoneMotionTrackBinaryHeader CreateModelBoneMotionTrackBinaryHeader(const Struct::SkeletalAnimationModelBoneMotionTrack& a_modelBoneMotionTrack) const;

        bool TryReadModelMeshBinaryData(const std::uint64_t& a_boneCount, Struct::SkeletalAnimationModelMesh& a_modelMesh, std::uint64_t& a_memoryReadOffset) const;

        void WriteModelMeshBinaryData(const Struct::SkeletalAnimationModelMesh& a_modelMesh, std::uint64_t& a_memoryWriteOffset) const;

        std::uint64_t CalculateModelMeshBinaryFileSize(const Struct::SkeletalAnimationModelMesh& a_modelMesh) const;

        std::uint64_t CalculateAssetFileSize(const Struct::SkeletalAnimationModelData& a_modelData) const;

        bool IsValidModelMesh(const Struct::SkeletalAnimationModelMesh& a_modelMesh, const std::uint64_t& a_boneCount) const;

        static constexpr std::uint64_t k_emptyBoneNameSize         = 0ULL;
        static constexpr std::uint64_t k_emptyMotionNameSize       = 0ULL;
        static constexpr std::uint64_t k_emptyBoneMotionTrackCount = 0ULL;
        static constexpr std::uint64_t k_emptyKeyFrameCount        = 0ULL;

        // 'S' = 0x53、'A' = 0x41のため、0x5341で"SA"を表す
        static constexpr std::uint16_t k_modelAssetTypeID = 0x5341U;

        static constexpr std::uint16_t k_modelAssetVersion = 1U;
    };
}