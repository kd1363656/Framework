#pragma once

namespace FWK::Struct
{
	struct CBSkeletalAnimationBoneHierarchy
	{
		TypeAlias::DescriptorIndex m_boneBufferSRVDescriptorIndex       = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		TypeAlias::DescriptorIndex m_boneMatrixBufferUAVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;

		std::uint32_t m_hierarchyDepth = Graphics::SkeletalAnimationModelRecord::k_initialMAXBoneHierarchyDepth;
		std::uint32_t m_boneCount      = Converter::SkeletalAnimationModelBinaryConverter::k_emptyBoneCount;
	};
}