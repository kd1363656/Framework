#pragma once

namespace FWK::Struct
{
    // Hi-Z作成用ComputeShader(HierarchicalDepthBuild_CS.hlsl)へ渡す定数バッファ
    // ※ HLSL側と必ず同じ並びにする
    struct CBHierarchicalDepthBuildPass final
    {
        static constexpr std::uint32_t k_initialValue = 0U;

        TypeAlias::DescriptorIndex m_sourceDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        std::uint32_t              m_isSourceDepthTexture  = k_initialValue;
        std::uint32_t              m_sourceWidth           = k_initialValue;
        std::uint32_t              m_sourceHeight          = k_initialValue;

        TypeAlias::DescriptorIndex m_destinationUAVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        std::uint32_t              m_padding                       = k_initialValue;
        std::uint32_t              m_destinationWidth              = k_initialValue;
        std::uint32_t              m_destinationHeight             = k_initialValue;
    };
}