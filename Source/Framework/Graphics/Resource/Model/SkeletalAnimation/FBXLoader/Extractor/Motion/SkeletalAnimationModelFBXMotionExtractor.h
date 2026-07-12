#pragma once

namespace FWK::Graphics
{
	class SkeletalAnimationModelFBXMotionExtractor final
	{
	public:

		 SkeletalAnimationModelFBXMotionExtractor() = default;
		~SkeletalAnimationModelFBXMotionExtractor() = default;

		bool ExtractModelMotionSequenceList(const std::unordered_map<const ufbx_node*, std::uint32_t>& a_boneNodeIndexMap, const ufbx_scene* a_fbxScene, std::vector<SkeletalAnimationModelRecord::ModelMotionSequence>& a_modelMotionSequenceList) const;

	private:

		SkeletalAnimationModelRecord::ModelKeyFrame CreateModelKeyFrame(const ufbx_baked_node* a_fbxBakedNode, const double& a_timeSecond) const;

		bool CreateModelBoneMotionTrack(const ufbx_baked_node*                                    a_fbxBakedNode,
										const double&                                             a_animationDurationSecond,
										const double&                                             a_animationFrameRate,
										const std::uint32_t                                       a_boneIndex,
										      SkeletalAnimationModelRecord::ModelBoneMotionTrack& a_modelBoneMotionTrack) const;

		bool CreateModelMotionSequenceFromBakedAnimation(const std::unordered_map<const ufbx_node*, std::uint32_t>& a_boneNodeIndexMap, 
			                                             const ufbx_scene*                                          a_fbxScene,
			                                             const ufbx_baked_anim*                                     a_fbxBakedAnimation, 
			                                                   SkeletalAnimationModelRecord::ModelMotionSequence&   a_modelMotionSequence) const;

		bool CreateModelMotionSequence(const std::unordered_map<const ufbx_node*, std::uint32_t>& a_boneNodeIndexMap,
									   const ufbx_scene*                                          a_fbxScene,
									   const ufbx_anim_stack*                                     a_fbxAnimationStack,
									         SkeletalAnimationModelRecord::ModelMotionSequence&   a_modelMotionSequence) const;


		TypeAlias::Math::Quaternion ConvertUFBXQuaternionToQuaternion(const ufbx_quat& a_fbxQuaternion) const;

		static constexpr double k_invalidAnimationFrameRate = 0.0;

		static constexpr std::uint64_t k_animationTerminalKeyFrameCount = 1ULL;
	};
}