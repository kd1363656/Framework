#pragma once

namespace FWK::Struct
{
    struct CBSkeletalAnimationVertexSkinningPerObject final
    {
        TypeAlias::DescriptorIndex m_sourceVertexBufferSRVDescriptorIndex  = Constant::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_bonePaletteBufferSRVDescriptorIndex   = Constant::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_boneMatrixBufferSRVDescriptorIndex    = Constant::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_skinnedVertexBufferUAVDescriptorIndex = Constant::k_invalidDescriptorIndex;

        std::uint32_t m_vertexCount = Constant::k_emptyModelVertexCount;
    };
}