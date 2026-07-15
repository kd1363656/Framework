#pragma once

namespace FWK::Struct
{
	struct  CBSkeletalAnimationLocalMatrix final
	{
		float m_blendWeight                    = Graphics::SkeletalAnimationPlayer::k_initialBlendWeight;
		float m_animationTimeSecond            = Graphics::SkeletalAnimationModelRecord::k_initialAnimationTimeSecond;
		float m_blendTargetAnimationTimeSecond = Graphics::SkeletalAnimationModelRecord::k_initialAnimationTimeSecond;

		TypeAlias::DescriptorIndex m_boneBufferSRVDescriptorIndex            = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		TypeAlias::DescriptorIndex m_motionSequenceBufferSRVDescriptorIndex  = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		TypeAlias::DescriptorIndex m_boneMotionTrackBufferSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		TypeAlias::DescriptorIndex m_keyFrameBufferSRVDescriptorIndex        = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		TypeAlias::DescriptorIndex m_boneMatrixBufferUAVDescriptorIndex      = Graphics::DescriptorHeap::k_invalidDescriptorIndex;

		std::uint32_t m_motionIndex            = Graphics::SkeletalAnimationPlayer::Animation::k_invalidMotionIndex;
		std::uint32_t m_blendTargetMotionIndex = Graphics::SkeletalAnimationPlayer::Animation::k_invalidMotionIndex;
		std::uint32_t m_boneCount              = Converter::SkeletalAnimationModelBinaryConverter::k_emptyBoneCount;
		std::uint32_t m_isBlending             = static_cast<std::uint32_t>(false);
	};
}