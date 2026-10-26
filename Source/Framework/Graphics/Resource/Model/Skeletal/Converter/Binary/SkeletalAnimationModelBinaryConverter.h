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

            std::uint64_t m_boneMotionTrackCount  = k_emptyBoneMotionTrackCount;
            std::uint64_t m_morphWeightTrackCount = k_emptyMorphWeightTrackCount;
        };

        struct ModelBoneMotionTrackBinaryHeader final
        {
            std::uint64_t m_keyFrameCount = k_emptyKeyFrameCount;

            std::uint32_t m_boneIndex = Graphics::SkeletalAnimationModelRecord::k_invalidBoneIndex;
        };

        struct ModelMorphTargetBinaryHeader final
        {
            std::uint64_t m_targetNameSize = k_emptyTargetNameSize;

            Enum::ModelMorphLODProtection m_lodProtection = Enum::ModelMorphLODProtection::Normal;

            std::uint32_t m_padding = k_initialMorphTargetPadding;
        };

        struct ModelMorphDataBinaryHeader final
        {
            std::uint64_t m_targetCount              = k_emptyMorphTargetCount;
            std::uint64_t m_vertexRangeCount         = k_emptyMorphVertexRangeCount;
            std::uint64_t m_vertexDeltaCount         = k_emptyMorphVertexDeltaCount;
            std::uint64_t m_lodTargetMorphErrorCount = k_emptyLODTargetMorphErrorCount;
        };

        struct ModelMorphWeightTrackBinaryHeader final
        {
            std::uint64_t m_targetNameSize = k_emptyTargetNameSize;
            std::uint64_t m_keyFrameCount  = k_emptyKeyFrameCount;
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

        bool TryReadModelMorphDataBinaryData(const std::size_t a_vertexCount, Struct::ModelMorphData& a_modelMorphData, std::uint64_t& a_memoryReadOffset) const;

        void WriteModelMorphDataBinaryData(const Struct::ModelMorphData& a_modelMorphData, std::uint64_t& a_memoryWriteOffset) const;

        std::uint64_t CalculateModelMorphDataBinaryFileSize(const Struct::ModelMorphData& a_modelMorphData) const;

        bool TryReadModelMorphWeightTrackListBinaryData(const std::uint64_t& a_morphWeightTrackCount, std::vector<Struct::ModelMorphWeightTrack>& a_modelMorphWeightTrackList, std::uint64_t& a_memoryReadOffset) const;

        void WriteModelMorphWeightTrackListBinaryData(const std::vector<Struct::ModelMorphWeightTrack>& a_modelMorphWeightTrackList, std::uint64_t& a_memoryWriteOffset) const;

        std::uint64_t CalculateModelMorphWeightTrackListBinaryFileSize(const std::vector<Struct::ModelMorphWeightTrack>& a_modelMorphWeightTrackList) const;

        bool IsValidModelMorphData(const Struct::ModelMorphData& a_modelMorphData, const std::size_t a_vertexCount, const std::size_t a_lodCount) const;

        std::uint64_t CalculateAssetFileSize(const Graphics::SkeletalAnimationModelRecord::ModelData& a_modelData) const;

        bool IsValidModelMesh(const Graphics::SkeletalAnimationModelRecord::ModelMesh& a_modelMesh, const std::uint64_t& a_boneCount) const;

        static constexpr std::uint64_t k_emptyBoneNameSize          = 0ULL;
        static constexpr std::uint64_t k_emptyMotionNameSize        = 0ULL;
        static constexpr std::uint64_t k_emptyBoneMotionTrackCount  = 0ULL;
        static constexpr std::uint64_t k_emptyKeyFrameCount         = 0ULL;
        static constexpr std::uint64_t k_emptyMorphWeightTrackCount = 0ULL;
        static constexpr std::uint64_t k_emptyTargetNameSize        = 0ULL;
        static constexpr std::uint64_t k_emptyMorphTargetCount      = 0ULL;
        static constexpr std::uint64_t k_emptyMorphVertexRangeCount = 0ULL;
        static constexpr std::uint64_t k_emptyMorphVertexDeltaCount = 0ULL;
        static constexpr std::uint64_t k_emptyLODTargetMorphErrorCount = 0ULL;

        static constexpr std::uint32_t k_initialMorphTargetPadding = 0U;

        static constexpr std::uint16_t k_modelAssetTypeID = 0x5341U;

        // Version2 : LOD一覧とMesh全体の囲み球を追加
        // Version3 : 遠距離用LODで継ぎ目をまたいだ簡略化を許可(LODの作り方の変更)
        // Version4 : 遠距離用LODでSloppy簡略化を使い、LODの最大数を9に増加(LODの作り方の変更)
        // Version5 : 簡略化で三角形が減らない場合は Permissive → Sloppy の順に試すように変更(LODの作り方の変更)
        // Version6 : Materialを.materialファイルへ分け、.materialファイルのパスだけを保存するように変更
        // Version7 : LODの簡略化で法線とUVのずれも誤差に含め、Sloppy簡略化をやめた(LODの作り方の変更)
        // Version8 : モーフ(ブレンドシェイプ)の移動量とウェイトのアニメーションを追加し、LODの簡略化でモーフの形も守るようにした
        //            LODにモーフの崩れ(m_morphError)を追加した(ModelMeshLODの並びの変更)
        // Version9 : LODごと・ターゲットごとのモーフの崩れ(ModelMorphData::m_lodTargetMorphErrorList)を追加した
        // Version10 : ターゲットごとのモーフの崩れを、補間後の位置との差で測るようにした(崩れの値の変更)
        // Version11 : 頂点ごとの覆われている層の数(m_lodCoveredLayerCountList)と、LODのずれの倍率(m_lodSurfaceDeviationScale)を追加した
        static constexpr std::uint16_t k_modelAssetVersion = 11U;
    };
}