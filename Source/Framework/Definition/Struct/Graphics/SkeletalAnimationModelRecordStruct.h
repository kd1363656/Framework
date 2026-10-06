#pragma once

namespace FWK::Struct
{
    struct SkeletalAnimationModelVertex final
    {
        TypeAlias::Math::Vector3 m_position = {};
        TypeAlias::Math::Vector3 m_normal   = {};
        TypeAlias::Math::Vector4 m_tangent  = {};
        TypeAlias::Math::Vector2 m_uv       = {};

        // 子の頂点が対応する最大4本のボーンから
        // それぞれどれだけ影響を受けるかを表す割合
        // 基本的には有効なWeightの合計が1.0Fになる
        TypeAlias::Math::Vector4 m_boneWeight = {};

        // 子の頂点に影響するBoneを
        // SkeletalAnimationModelMesh::m_bonePaletteListの何番から取得するかを表す
        std::uint32_t m_bonePaletteIndexZero  = Constant::k_invalidPaletteIndex;
        std::uint32_t m_bonePaletteIndexOne   = Constant::k_invalidPaletteIndex;
        std::uint32_t m_bonePaletteIndexTwo   = Constant::k_invalidPaletteIndex;
        std::uint32_t m_bonePaletteIndexThree = Constant::k_invalidPaletteIndex;
    };

    struct SkeletalAnimationModelBonePaletteElement final
    {
        // バンドポーズ時点ですでにModel空間へ配置されている頂点から
        // BindPose時のBone変換を取り除くためのMatrix
        // Skinning時にCurrentGlobalBoneMatrixと組み合わせて使用する
        TypeAlias::Math::Matrix m_inverseBindPoseMatrix = TypeAlias::Math::Matrix::Identity;

        std::uint32_t m_boneIndex = Constant::k_invalidBoneIndex;
    };

    struct SkeletalAnimationModelMeshRuntimeData final : public Struct::ModelMeshRuntimeDataBase
    {
        Graphics::StaticStructuredBuffer m_bonePaletteBuffer = {};
    };

    struct SkeletalAnimationModelMesh final
    {
         SkeletalAnimationModelMesh() = default;
        ~SkeletalAnimationModelMesh() = default;

        SkeletalAnimationModelMesh(const SkeletalAnimationModelMesh&)           = delete;
        SkeletalAnimationModelMesh(      SkeletalAnimationModelMesh&&) noexcept = default;

        SkeletalAnimationModelMesh& operator=(const SkeletalAnimationModelMesh&)           = delete;
        SkeletalAnimationModelMesh& operator=(      SkeletalAnimationModelMesh&&) noexcept = default;

        std::vector<SkeletalAnimationModelVertex> m_modelVertexList = {};

        std::vector<SkeletalAnimationModelBonePaletteElement> m_bonePaletteList = {};

        std::vector<std::uint32_t> m_indexList = {};

        Struct::ModelMaterial m_modelMaterial = {};

        Struct::ModelMeshletData m_modelMeshletData = {};

        SkeletalAnimationModelMeshRuntimeData m_modelMeshRuntimeData = {};
    };

    struct SkeletalAnimationModelBone final
    {
        std::wstring m_boneName = {};

        TypeAlias::Math::Matrix m_bindPoseLocalMatrix = TypeAlias::Math::Matrix::Identity;

        std::uint32_t m_parentBoneIndex = Constant::k_invalidBoneIndex;
    };

    struct SkeletalAnimationModelKeyFrame final
    {
        TypeAlias::Math::Vector3    m_scale       = TypeAlias::Math::Vector3::One;
        TypeAlias::Math::Quaternion m_rotation    = TypeAlias::Math::Quaternion::Identity;
        TypeAlias::Math::Vector3    m_translation = TypeAlias::Math::Vector3::Zero;

        float m_timeSecond = Constant::k_initialAnimationTimeSecond;
    };

    struct SkeletalAnimationModelBoneMotionTrack final
    {
        std::vector<SkeletalAnimationModelKeyFrame> m_keyFrameList = {};

        std::uint32_t m_boneIndex = Constant::k_invalidBoneIndex;
    };

    struct SkeletalAnimationModelMotionSequence final
    {
        std::vector<SkeletalAnimationModelBoneMotionTrack> m_boneMotionTrackList = {};

        std::wstring m_motionName = {};

        float m_durationSecond = Constant::k_initialAnimationDurationSecond;

        float m_frameRate = Constant::k_defaultAnimationFrameRate;
    };

    struct SkeletalAnimationModelData final
    {
         SkeletalAnimationModelData() = default;
        ~SkeletalAnimationModelData() = default;

        SkeletalAnimationModelData(const SkeletalAnimationModelData&)           = delete;
        SkeletalAnimationModelData(      SkeletalAnimationModelData&&) noexcept = default;

        SkeletalAnimationModelData& operator=(const SkeletalAnimationModelData&)           = delete;
        SkeletalAnimationModelData& operator=(      SkeletalAnimationModelData&&) noexcept = default;

        std::vector<SkeletalAnimationModelMesh> m_modelMeshList = {};

        std::vector<SkeletalAnimationModelBone> m_boneList = {};

        std::vector<SkeletalAnimationModelMotionSequence> m_motionSequenceList = {};
    };
}