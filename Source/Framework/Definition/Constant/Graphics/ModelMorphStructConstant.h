#pragma once

namespace FWK::Constant
{
    inline constexpr std::uint32_t k_invalidModelMorphTargetIndex        = std::numeric_limits<std::uint32_t>::max();
    inline constexpr std::uint32_t k_invalidModelMorphSourceVertexIndex  = std::numeric_limits<std::uint32_t>::max();
    inline constexpr std::uint32_t k_initialModelMorphVertexDeltaOffset  = 0U;
    inline constexpr std::uint32_t k_initialModelMorphVertexDeltaCount   = 0U;
    inline constexpr std::uint32_t k_initialModelMorphVertexDeltaPadding = 0U;

    inline constexpr std::uint32_t k_initialSBModelMorphVertexDeltaValue = 0U;

    inline constexpr float k_initialModelMorphWeight           = 0.0F;
    inline constexpr float k_initialModelMorphWeightTimeSecond = 0.0F;
}