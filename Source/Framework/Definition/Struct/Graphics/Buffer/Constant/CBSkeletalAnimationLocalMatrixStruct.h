#pragma once

namespace FWK::Struct
{
	struct  CBSkeletalAnimationLocalMatrix final
	{
		TypeAlias::DescriptorIndex m_boneBufferSRVDescriptorIndex            = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		TypeAlias::DescriptorIndex m_motionSequenceBufferSRVDescriptorIndex  = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		TypeAlias::DescriptorIndex m_boneMotionTrackBufferSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		TypeAlias::DescriptorIndex m_keyFrameBufferSRVDescirptorIndex        = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		TypeAlias::DescriptorIndex m_boneMatrixBufferUAVDescirptorIndex      = Graphics::DescriptorHeap::k_invalidDescriptorIndex;

		float m_blendWeight                    = Graphics::SkeletalAnimationPlayer::k_initialBlendWeight;
		float m_animationTimeSecond            = Graphics::SkeletalAnimationModelRecord::k_initialAnimationTimeSecond;
		float m_blendTargetAnimationTimeSecond = Graphics::SkeletalAnimationModelRecord::k_initialAnimationTimeSecond;

		std::uint32_t m_motionIndex            = Graphics::SkeletalAnimationPlayer::Animation::k_invalidMotionIndex;
		std::uint32_t m_blendTargetMotionIndex = Graphics::SkeletalAnimationPlayer::Animation::k_invalidMotionIndex;
		std::uint32_t m_boneCount              = Constant::k_initialBoneCount;

		std::uint32_t m_isBlending = static_cast<std::uint32_t>(Graphics::SkeletalAnimationPlayer::k_initialIsBlending);
	};
}