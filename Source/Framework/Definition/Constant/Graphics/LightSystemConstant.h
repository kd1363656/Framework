#pragma once

namespace FWK::Constant
{
    inline constexpr TypeAlias::Math::Vector3 k_defaultDirectionalLightDirection = {  0.50F, -1.0F, 0.50F };
    inline constexpr TypeAlias::Math::Vector3 k_defaultDirectionalLightColor     = {  1.0F,   1.0F, 1.0F };

    inline constexpr TypeAlias::Math::Vector3 k_defaultAmbientLightColor = { 1.0F, 1.0F, 1.0F };

    inline constexpr float k_defaultDirectionalLightIntensity = 1.0F;
    inline constexpr float k_defaultAmbientLightIntensity     = 0.25F;

    static constexpr float k_epsilon = 0.000001F;
}