#pragma once

namespace FWK::Constant
{
    inline const std::filesystem::path k_skeletalAnimationModelSpringBoneSettingExtension = ".spring";

    inline constexpr TypeAlias::Math::Vector3 k_defaultModelSpringBoneGravityDirection = { 0.0F, -1.0F, 0.0F };

    inline constexpr TypeAlias::Math::Vector3 k_defaultModelSpringBoneColliderOffset     = { 0.0F, 0.0F, 0.0F };
    inline constexpr TypeAlias::Math::Vector3 k_defaultModelSpringBoneColliderTailOffset = { 0.0F, 0.0F, 0.0F };

    inline constexpr float k_defaultModelSpringBoneStiffness                = 4.0F;
    inline constexpr float k_defaultModelSpringBoneDrag                     = 0.4F;
    inline constexpr float k_defaultModelSpringBoneGravityPower             = 0.0F;
    inline constexpr float k_defaultModelSpringBoneHitRadius                = 0.02F;
    inline constexpr float k_defaultModelSpringBoneMAXAngularVelocityDegree = 720.0F;
    inline constexpr float k_defaultModelSpringBoneEndLength                = 0.0F;
    inline constexpr float k_defaultModelSpringBoneAdjacentLinkStiffness    = 600.0F;
    inline constexpr float k_defaultModelSpringBoneColliderRadius           = 0.05F;

    inline constexpr std::uint32_t k_defaultModelSpringBonePrewarmIterationCount = 60U;
    inline constexpr std::uint32_t k_initialModelSpringBoneSettingRevision       = 0U;
}