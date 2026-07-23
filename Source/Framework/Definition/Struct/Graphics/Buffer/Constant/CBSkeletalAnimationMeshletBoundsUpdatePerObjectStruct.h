#pragma once

namespace FWK::Struct
{
	struct CBSkeletalAnimationMeshletBoundsUpdatePerObject final
	{
		TypeAlias::DescriptorIndex m_vertexBufferSRVDescriptorIndex            = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		TypeAlias::DescriptorIndex m_meshletBufferSRVDescriptorIndex           = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		TypeAlias::DescriptorIndex m_uniqueVertexIndexBufferSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		TypeAlias::DescriptorIndex m_primitiveIndexBufferSRVDescriptorIndex    = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		TypeAlias::DescriptorIndex m_meshletBoundsBufferUAVDescriptorIndex     = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
	};
}