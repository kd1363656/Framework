#pragma once

namespace FWK::Struct
{
	struct CBSkeletalAnimationVertexSkinningPerObject final
	{
		TypeAlias::DescriptorIndex m_sourceVertexBufferSRVDescriptorIndex  = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		TypeAlias::DescriptorIndex m_bonePaletteBufferSRVDescriptorIndex   = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		TypeAlias::DescriptorIndex m_boneMatrixBufferSRVDescriptorIndex    = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		TypeAlias::DescriptorIndex m_skinnedVertexBufferUAVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;

		std::uint32_t m_vertexCount = Constant::k_emptyModelVertexCount;
	};
}