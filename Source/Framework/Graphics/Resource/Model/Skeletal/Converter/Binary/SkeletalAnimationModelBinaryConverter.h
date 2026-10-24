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
            std::uint64_t m_boneCount           = Constant::k_emptyBoneCount;
            std::uint64_t m_motionSequenceCount = Constant::k_emptyMotionSequenceCount;
        };

        struct ModelBoneBinaryHeader final
        {
            std::uint64_t m_boneNameSize = k_emptyBoneNameSize;

            std::uint32_t m_parentBoneIndex = Graphics::SkeletalAnimationModelRecord::k_invalidBoneIndex;
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

            std::uint32_t m_boneIndex = Graphics::SkeletalAnimationModelRecord::k_invalidBoneIndex;
        };

    public:

         SkeletalAnimationModelBinaryConverter()          = default;
        ~SkeletalAnimationModelBinaryConverter() override = default;

        bool LoadAsset(const std::filesystem::path& a_filePath,       Graphics::SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord);
        bool SaveAsset(const std::filesystem::path& a_filePath, const Graphics::SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord);

    private:

        void FailLoadAsset(Graphics::SkeletalAnimationModelRecord::ModelData& a_modelData);

        ModelBinaryHeader CreateModelBinaryHeader(const Graphics::SkeletalAnimationModelRecord::ModelData& a_modelData, const std::uint64_t& a_fileSize) const;

        ModelBoneBinaryHeader CreateModelBoneBinaryHeader(const Graphics::SkeletalAnimationModelRecord::ModelBone& a_modelBone) const;

        ModelMotionSequenceBinaryHeader CreateMotionSequenceBinaryHeader(const Graphics::SkeletalAnimationModelRecord::ModelMotionSequence& a_modelMotionSequence) const;

        ModelBoneMotionTrackBinaryHeader CreateModelBoneMotionTrackBinaryHeader(const Graphics::SkeletalAnimationModelRecord::ModelBoneMotionTrack& a_modelBoneMotionTrack) const;

        bool TryReadModelMeshBinaryData(const std::uint64_t& a_boneCount, Graphics::SkeletalAnimationModelRecord::ModelMesh& a_modelMesh, std::uint64_t& a_memoryReadOffset) const;

        void WriteModelMeshBinaryData(const Graphics::SkeletalAnimationModelRecord::ModelMesh& a_modelMesh, std::uint64_t& a_memoryWriteOffset) const;

        std::uint64_t CalculateModelMeshBinaryFileSize(const Graphics::SkeletalAnimationModelRecord::ModelMesh& a_modelMesh) const;

        std::uint64_t CalculateAssetFileSize(const Graphics::SkeletalAnimationModelRecord::ModelData& a_modelData) const;

        bool IsValidModelMesh(const Graphics::SkeletalAnimationModelRecord::ModelMesh& a_modelMesh, const std::uint64_t& a_boneCount) const;

        static constexpr std::uint64_t k_emptyBoneNameSize         = 0ULL;
        static constexpr std::uint64_t k_emptyMotionNameSize       = 0ULL;
        static constexpr std::uint64_t k_emptyBoneMotionTrackCount = 0ULL;
        static constexpr std::uint64_t k_emptyKeyFrameCount        = 0ULL;

        // SkeletalModelのバイナリーファイル用拡張子
        // StaticModelと同じFBXから作っても別ファイルになるように、モデルの種類ごとに拡張子を分ける
        inline static const std::filesystem::path k_skeletalModelAssetExtension = ".skeletalModel";

        static constexpr std::uint16_t k_modelAssetTypeID = 0x5341U;

        // Version2 : LOD一覧とMesh全体の囲み球を追加
        // Version3 : 遠距離用LODで継ぎ目をまたいだ簡略化を許可(LODの作り方の変更)
        // Version4 : 遠距離用LODでSloppy簡略化を使い、LODの最大数を9に増加(LODの作り方の変更)
        // Version5 : 簡略化で三角形が減らない場合は Permissive → Sloppy の順に試すように変更(LODの作り方の変更)
        // Version6 : Materialを.materialファイルへ分け、.materialファイルのパスだけを保存するように変更
        // Version7 : LODの簡略化で法線とUVのずれも誤差に含め、Sloppy簡略化をやめた(LODの作り方の変更)
        static constexpr std::uint16_t k_modelAssetVersion = 7U;
    };
}