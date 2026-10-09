#pragma once

namespace FWK::Graphics
{
    class SkeletalAnimationModelFBXMotionExtractor final
    {
    public:

         SkeletalAnimationModelFBXMotionExtractor() = default;
        ~SkeletalAnimationModelFBXMotionExtractor() = default;

        bool ExtractModelMotionSequenceList(const std::unordered_map<const ufbx_node*, std::uint32_t>& a_boneNodeIndexMap, const ufbx_scene* a_fbxScene, std::vector<Struct::SkeletalAnimationModelMotionSequence>& a_modelMotionSequenceList) const;

    private:

        Struct::SkeletalAnimationModelKeyFrame CreateModelKeyFrame(const double& a_timeSecond, const ufbx_baked_node* a_fbxBakedNode) const;

        bool CreateModelBoneMotionTrack(const double&                                        a_animationDurationSecond,
                                        const double&                                        a_animationFrameRate,
                                        const ufbx_baked_node*                               a_fbxBakedNode,
                                        const std::uint32_t                                  a_boneIndex,
                                              Struct::SkeletalAnimationModelBoneMotionTrack& a_modelBoneMotionTrack) const;

        bool CreateModelMotionSequenceFromBakedAnimation(const std::unordered_map<const ufbx_node*, std::uint32_t>& a_boneNodeIndexMap,
                                                         const ufbx_scene*                                          a_fbxScene,
                                                         const ufbx_baked_anim*                                     a_fbxBakedAnimation,
                                                               Struct::SkeletalAnimationModelMotionSequence&        a_modelMotionSequence) const;

        bool CreateModelMotionSequence(const std::unordered_map<const ufbx_node*, std::uint32_t>& a_boneNodeIndexMap,
                                       const ufbx_scene*                                          a_fbxScene,
                                       const ufbx_anim_stack*                                     a_fbxAnimationStack,
                                             Struct::SkeletalAnimationModelMotionSequence&        a_modelMotionSequence) const;

        TypeAlias::Math::Quaternion ConvertUFBXQuaternionToQuaternion(const ufbx_quat& a_fbxQuaternion) const;

        static constexpr double k_invalidAnimationFrameRate = 0.0;

        static constexpr std::uint64_t k_animationTerminalKeyFrameCount = 1ULL;
    };
}