#pragma once

namespace FWK::Constant
{
    inline constexpr TypeAlias::Math::Color k_defaultRitchToonMaterialBaseColorFactor = { 1.0F,  1.0F,  1.0F,  1.0F };
    inline constexpr TypeAlias::Math::Color k_defaultRitchToonMaterialShadowColor     = { 0.82F, 0.76F, 0.85F, 1.0F };
    inline constexpr TypeAlias::Math::Color k_defaultRitchToonMaterialMatcapColor     = { 0.3F,  0.3F,  0.3F,  1.0F };
    inline constexpr TypeAlias::Math::Color k_defaultRitchToonMaterialSpecularColor   = { 0.0F,  0.0F,  0.0F,  1.0F };
    inline constexpr TypeAlias::Math::Color k_defaultRitchToonMaterialEmissionColor   = { 0.0F,  0.0F,  0.0F,  1.0F };
    inline constexpr TypeAlias::Math::Color k_defaultRitchToonMaterialOutlineColor    = { 0.45F, 0.4F,  0.45F, 1.0F };

    inline constexpr float k_defaultRitchToonMaterialShadowBorder   = 0.5F;
    inline constexpr float k_defaultRitchToonMaterialShadowBlur     = 0.02F;
    inline constexpr float k_defaultRitchToonMaterialNormalStrength = 1.0F;
    inline constexpr float k_defaultRitchToonMaterialSmoothness     = 0.8F;
    inline constexpr float k_defaultRitchToonMaterialOutlineWidth   = 0.005F;

    inline constexpr float k_defaultRitchToonMaterialEmissionBaseColorBlend = 0.0F;
    inline constexpr float k_defaultRitchToonMaterialEmissionReplaceRate    = 0.0F;

    inline constexpr float k_initialRitchToonMaterialPadding = 0.0F;
}