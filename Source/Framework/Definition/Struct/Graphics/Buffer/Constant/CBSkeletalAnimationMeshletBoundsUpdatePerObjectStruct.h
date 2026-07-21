#pragma once

namespace FWK::Struct
{
	struct CBSkeletalAnimationMeshletBoundsUpdatePerObject final
	{
		TypeAlias::DescriptorIndex m_skinnedVertexBufferSRVDesccriptorIndex       = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		TypeAlias::DescriptorIndex m_meshletBufferSRVDesccriptorIndex             = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		TypeAlias::DescriptorIndex m_uniqueVertexIndexBufferSRVDesccriptorIndex   = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		TypeAlias::DescriptorIndex m_skinnedMeshletBoundsBufferUAVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
	};
}