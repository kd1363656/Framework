#pragma once

namespace FWK::Struct
{
    struct RCModelDrawItem final
    {
        static constexpr std::uint32_t k_invalidIndex = std::numeric_limits<std::uint32_t>::max();

        std::uint32_t m_objectIndex   = k_invalidIndex;
        std::uint32_t m_meshIndex     = k_invalidIndex;
        std::uint32_t m_materialIndex = k_invalidIndex;
    };

    struct RCModelTable final
    {
        TypeAlias::DescriptorIndex m_objectTableSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_meshTableSRVDescriptorIndex   = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
    };

    struct RCModelMaterialTable final
    {
        TypeAlias::DescriptorIndex m_materialTableSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
    };
}