#pragma once

namespace FWK::Constant
{
    // StandardLit / StandardUnLitMaterialの初期値
    // FBXにMaterialの値が入っていない場合もこの値を使う
    inline constexpr TypeAlias::Math::Color k_defaultStandardMaterialBaseColorFactor = { 1.0F, 1.0F, 1.0F, 1.0F };

    static constexpr float k_defaultStandardMaterialRoughnessFactor = 1.0F;
    static constexpr float k_defaultStandardMaterialMetallicFactor  = 0.0F;
}