#pragma once

namespace FWK::Struct
{
    // Hi-Z作成用ComputeShader(HierarchicalDepthBuild_CS.hlsl)へ渡す定数バッファ
    // ※ HLSL側と必ず同じ並びにする
    struct CBHierarchicalDepthBuildPass final
    {
        TypeAlias::DescriptorIndex m_sourceDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        std::uint32_t              m_isSourceDepthTexture  = Constant::k_initialCBHierarchicalDepthBuildPassValue;
        std::uint32_t              m_sourceWidth           = Constant::k_initialCBHierarchicalDepthBuildPassValue;
        std::uint32_t              m_sourceHeight          = Constant::k_initialCBHierarchicalDepthBuildPassValue;

        TypeAlias::DescriptorIndex m_destinationUAVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        std::uint32_t              m_padding                       = Constant::k_initialCBHierarchicalDepthBuildPassValue;
        std::uint32_t              m_destinationWidth              = Constant::k_initialCBHierarchicalDepthBuildPassValue;
        std::uint32_t              m_destinationHeight             = Constant::k_initialCBHierarchicalDepthBuildPassValue;
    };
}