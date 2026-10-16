#pragma once

namespace FWK::Struct
{
    struct CBSkeletalAnimationMeshletBoundsUpdatePerObject final
    {
        TypeAlias::DescriptorIndex m_vertexBufferSRVDescriptorIndex            = Constant::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_meshletBufferSRVDescriptorIndex           = Constant::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_uniqueVertexIndexBufferSRVDescriptorIndex = Constant::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_primitiveIndexBufferSRVDescriptorIndex    = Constant::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_meshletBoundsBufferUAVDescriptorIndex     = Constant::k_invalidDescriptorIndex;
    };
}