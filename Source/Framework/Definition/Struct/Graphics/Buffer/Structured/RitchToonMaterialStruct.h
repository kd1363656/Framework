#pragma once

namespace FWK::Struct
{
    // RitchToonMaterialがシェーダーへ送るデータ(トゥーン描画に必要なものだけを持つ)
    // Vector3の後ろにfloatを1つ並べて、16byte(HLSLのfloat4 1つ分)ごとに区切っている
    // ※ HLSL側(Shader/Material/RitchToon/RitchToonMaterial.hlsli)と必ず同じ並びにする
    struct SBRitchToonMaterial final
    {
        TypeAlias::Math::Color m_baseColorFactor = Constant::k_defaultRitchToonMaterialBaseColorFactor;

        TypeAlias::Math::Vector3 m_shadowColor  = {};
        float                    m_shadowBorder = Constant::k_defaultRitchToonMaterialShadowBorder;

        TypeAlias::Math::Vector3 m_matcapColor = {};
        float                    m_shadowBlur  = Constant::k_defaultRitchToonMaterialShadowBlur;

        TypeAlias::Math::Vector3 m_specularColor  = {};
        float                    m_normalStrength = Constant::k_defaultRitchToonMaterialNormalStrength;

        TypeAlias::Math::Vector3 m_emissionColor = {};
        float                    m_smoothness    = Constant::k_defaultRitchToonMaterialSmoothness;

        TypeAlias::Math::Vector3 m_outlineColor = {};
        float                    m_outlineWidth = Constant::k_defaultRitchToonMaterialOutlineWidth;

        TypeAlias::DescriptorIndex m_baseColorTextureSRVDescriptorIndex  = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_normalTextureSRVDescriptorIndex     = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_shadowMaskTextureSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_matcapTextureSRVDescriptorIndex     = Graphics::DescriptorHeap::k_invalidDescriptorIndex;

        TypeAlias::DescriptorIndex m_matcapMaskTextureSRVDescriptorIndex   = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_metallicMaskTextureSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_emissionMaskTextureSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_outlineMaskTextureSRVDescriptorIndex  = Graphics::DescriptorHeap::k_invalidDescriptorIndex;

        // m_padding～ : HLSL側と大きさを16byte単位で揃えるための詰め物
        float m_emissionBaseColorBlend = Constant::k_defaultRitchToonMaterialEmissionBaseColorBlend;
        float m_emissionReplaceRate    = Constant::k_defaultRitchToonMaterialEmissionReplaceRate;
        float m_padding                = Constant::k_initialRitchToonMaterialPadding;
        float m_padding2               = Constant::k_initialRitchToonMaterialPadding;
    };

    static_assert(sizeof(SBRitchToonMaterial) == 144ULL, "SBRitchToonMaterialのサイズがHLSL側と一致しません。");
}