#pragma once

namespace FWK::Struct
{
	struct CBSkeletalAnimationMeshletBoundsUpdatePerObject final
	{
		TypeAlias::DescriptorIndex m_skinnedVertexBufferSRVDescriptorIndex        = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		TypeAlias::DescriptorIndex m_meshletBufferSRVDescriptorIndex              = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		TypeAlias::DescriptorIndex m_uniqueVertexIndexBufferSRVDescriptorIndex    = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		TypeAlias::DescriptorIndex m_skinnedMeshletBoundsBufferUAVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
	};
}