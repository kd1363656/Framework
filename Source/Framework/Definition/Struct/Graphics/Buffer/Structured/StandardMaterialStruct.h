#pragma once

namespace FWK::Struct
{
    // StandardLitMaterialがシェーダーへ送るデータ(Lit描画に必要なものだけを持つ)
    // ※ HLSL側(Shader/Material/Standard/Lit/StandardLitMaterial.hlsli)と必ず同じ並びにする
    struct SBStandardLitMaterial final
    {
        TypeAlias::Math::Color m_baseColorFactor = Constant::k_defaultStandardMaterialBaseColorFactor;

        float                      m_roughnessFactor                    = Constant::k_defaultStandardMaterialRoughnessFactor;
        float                      m_metallicFactor                     = Constant::k_defaultStandardMaterialMetallicFactor;
        TypeAlias::DescriptorIndex m_baseColorTextureSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_normalTextureSRVDescriptorIndex    = Graphics::DescriptorHeap::k_invalidDescriptorIndex;

        TypeAlias::DescriptorIndex m_metallicTextureSRVDescriptorIndex  = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_roughnessTextureSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        std::uint32_t              m_padding                            = Constant::k_initialStandardLitMaterialPadding;
        std::uint32_t              m_padding2                           = Constant::k_initialStandardLitMaterialPadding;
    };

    // StandardUnLitMaterialがシェーダーへ送るデータ(ライティングしないため、ベースカラーだけを持つ)
    // ※ HLSL側(Shader/Material/Standard/UnLit/StandardUnLitMaterial.hlsli)と必ず同じ並びにする
    struct SBStandardUnLitMaterial final
    {
        TypeAlias::Math::Color m_baseColorFactor = Constant::k_defaultStandardMaterialBaseColorFactor;

        TypeAlias::DescriptorIndex m_baseColorTextureSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        std::uint32_t              m_padding                            = Constant::k_initialStandardUnLitMaterialPadding;
        std::uint32_t              m_padding2                           = Constant::k_initialStandardUnLitMaterialPadding;
        std::uint32_t              m_padding3                           = Constant::k_initialStandardUnLitMaterialPadding;
    };

    static_assert(sizeof(SBStandardLitMaterial)   == 48ULL, "SBStandardLitMaterialのサイズがHLSL側と一致しません。");
    static_assert(sizeof(SBStandardUnLitMaterial) == 32ULL, "SBStandardUnLitMaterialのサイズがHLSL側と一致しません。");
}