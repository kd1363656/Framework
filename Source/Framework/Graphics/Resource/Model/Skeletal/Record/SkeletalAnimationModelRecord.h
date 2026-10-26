#pragma once

namespace FWK::Graphics
{
    class SkeletalAnimationBakedSkinning;
}

namespace FWK::Graphics
{
    class SkeletalAnimationModelRecord final : public AssetRecordBase
    {
    public:

        struct ModelVertex final
        {
            TypeAlias::Math::Vector3 m_position = {};
            TypeAlias::Math::Vector3 m_normal   = {};
            TypeAlias::Math::Vector4 m_tangent  = {};
            TypeAlias::Math::Vector2 m_uv       = {};

            TypeAlias::Math::Vector4 m_boneWeight = {};

            std::uint32_t m_bonePaletteIndexZero  = k_invalidPaletteIndex;
            std::uint32_t m_bonePaletteIndexOne   = k_invalidPaletteIndex;
            std::uint32_t m_bonePaletteIndexTwo   = k_invalidPaletteIndex;
            std::uint32_t m_bonePaletteIndexThree = k_invalidPaletteIndex;
        };

        struct ModelBonePaletteElement final
        {
            TypeAlias::Math::Matrix m_inverseBindPoseMatrix = TypeAlias::Math::Matrix::Identity;

            std::uint32_t m_boneIndex = k_invalidBoneIndex;
        };

        struct ModelMeshRuntimeData final : public Struct::ModelMeshRuntimeDataBase
        {
            Graphics::StaticStructuredBuffer m_bonePaletteBuffer = {};

            // モーフを持つMeshだけ作成する(持たないMeshでは作成しない)
            // m_morphVertexRangeBuffer    : 頂点ごとの移動量の範囲(Struct::ModelMorphVertexRange)
            // m_morphDeltaBuffer          : GPU用に詰めた移動量(Struct::SBModelMorphVertexDelta)
            // m_morphChannelListBuffer    : このMeshが使うチャンネル番号の一覧(LODを選ぶときに、効いているウェイトを調べるために使う)
            // m_morphLODTargetErrorBuffer : LODごと・ターゲットごとのモーフの崩れ(ModelMorphData::m_lodTargetMorphErrorList)
            Graphics::StaticStructuredBuffer m_morphVertexRangeBuffer    = {};
            Graphics::StaticStructuredBuffer m_morphDeltaBuffer          = {};
            Graphics::StaticStructuredBuffer m_morphChannelListBuffer    = {};
            Graphics::StaticStructuredBuffer m_morphLODTargetErrorBuffer = {};
        };

        struct ModelMesh final
        {
             ModelMesh() = default;
            ~ModelMesh() = default;

            ModelMesh(const ModelMesh&)           = delete;
            ModelMesh(      ModelMesh&&) noexcept = default;

            ModelMesh& operator=(const ModelMesh&)           = delete;
            ModelMesh& operator=(      ModelMesh&&) noexcept = default;

            std::vector<ModelVertex> m_modelVertexList = {};

            std::vector<ModelBonePaletteElement> m_bonePaletteList = {};

            std::vector<std::uint32_t> m_indexList = {};

            Struct::ModelMaterial m_modelMaterial = {};

            Struct::ModelMeshletData m_modelMeshletData = {};

            Struct::ModelMorphData m_modelMorphData = {};

            ModelMeshRuntimeData m_modelMeshRuntimeData = {};
        };

        struct ModelBone final
        {
            std::wstring m_boneName = {};

            TypeAlias::Math::Matrix m_bindPoseLocalMatrix = TypeAlias::Math::Matrix::Identity;

            std::uint32_t m_parentBoneIndex = k_invalidBoneIndex;
        };

        struct ModelKeyFrame final
        {
            TypeAlias::Math::Vector3    m_scale       = TypeAlias::Math::Vector3::One;
            TypeAlias::Math::Quaternion m_rotation    = TypeAlias::Math::Quaternion::Identity;
            TypeAlias::Math::Vector3    m_translation = TypeAlias::Math::Vector3::Zero;

            float m_timeSecond = Constant::k_initialAnimationTimeSecond;
        };

        struct ModelBoneMotionTrack final
        {
            std::vector<ModelKeyFrame> m_keyFrameList = {};

            std::uint32_t m_boneIndex = k_invalidBoneIndex;
        };

        struct ModelMotionSequence final
        {
            std::vector<ModelBoneMotionTrack> m_boneMotionTrackList = {};

            std::vector<Struct::ModelMorphWeightTrack> m_morphWeightTrackList = {};

            std::wstring m_motionName = {};

            float m_durationSecond = Constant::k_initialAnimationDurationSecond;

            float m_frameRate = Constant::k_defaultAnimationFrameRate;
        };

        struct ModelData final
        {
             ModelData() = default;
            ~ModelData() = default;

            ModelData(const ModelData&)           = delete;
            ModelData(      ModelData&&) noexcept = default;

            ModelData& operator=(const ModelData&)           = delete;
            ModelData& operator=(      ModelData&&) noexcept = default;

            std::vector<ModelMesh> m_modelMeshList = {};

            std::vector<ModelBone> m_boneList = {};

            std::vector<ModelMotionSequence> m_motionSequenceList = {};

            // モーフのチャンネル名の一覧(同じ名前のターゲットをモデル全体で1つにまとめた物、番号 = チャンネル番号)
            // ※ 読み込んだ後に作り、バイナリーファイルには保存しない
            std::vector<std::wstring> m_morphChannelNameList = {};
        };

    public:

         SkeletalAnimationModelRecord()          = default;
        ~SkeletalAnimationModelRecord() override = default;

        bool ReserveRelease(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext) override;

        void SetModelData(ModelData&& a_set) { m_modelData = std::move(a_set); }

        // GPUスキニング(大量のキャラクター向け)で使う、事前計算したアニメーション
        // 初めて使われるときに作成し、このモデルを使う全ての描画で共有する
        void SetBakedSkinning(const std::shared_ptr<SkeletalAnimationBakedSkinning>& a_set) { m_bakedSkinning = a_set; }

        void SetMorphSetting(SkeletalAnimationModelMorphSetting&& a_set) { m_morphSetting = std::move(a_set); }

        void SetSpringBoneSetting(SkeletalAnimationModelSpringBoneSetting&& a_set) { m_springBoneSetting = std::move(a_set); }

        // 名前が一致するモーフのチャンネル番号を探す(無い場合はConstant::k_invalidModelMorphTargetIndex)
        std::uint32_t FindVALMorphChannelIndex(const std::wstring& a_channelName) const;

        const auto& GetREFModelData() const { return m_modelData; }

        const auto& GetREFBakedSkinning() const { return m_bakedSkinning; }

        const auto& GetREFMorphSetting() const { return m_morphSetting; }

        const auto& GetREFSpringBoneSetting() const { return m_springBoneSetting; }

        auto& GetMutableREFModelData() { return m_modelData; }

        auto& GetMutableREFMorphSetting() { return m_morphSetting; }

        auto& GetMutableREFSpringBoneSetting() { return m_springBoneSetting; }

        static constexpr std::uint32_t k_invalidBoneIndex    = std::numeric_limits<std::uint32_t>::max();
        static constexpr std::uint32_t k_invalidPaletteIndex = std::numeric_limits<std::uint32_t>::max();

    private:

        ModelData m_modelData = {};

        SkeletalAnimationModelMorphSetting m_morphSetting = {};

        SkeletalAnimationModelSpringBoneSetting m_springBoneSetting = {};

        std::shared_ptr<SkeletalAnimationBakedSkinning> m_bakedSkinning = nullptr;
    };
}