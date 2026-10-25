#pragma once

namespace FWK::Constant
{
    inline constexpr std::uint32_t k_initialMeshletVertexOffset   = 0U;
    inline constexpr std::uint32_t k_initialMeshletTriangleOffset = 0U;
    inline constexpr std::uint32_t k_initialMeshletVertexCount    = 0U;
    inline constexpr std::uint32_t k_initialMeshletTriangleCount  = 0U;

    inline constexpr float k_initialMeshletBoundsRadius     = 0.0F;
    inline constexpr float k_initialMeshletBoundsConeCutoff = 0.0F;
    inline constexpr float k_initialMeshletBoundsPadding    = 0.0F;

    inline constexpr std::uint32_t k_initialMeshLODMeshletOffset = 0U;
    inline constexpr std::uint32_t k_initialMeshLODMeshletCount  = 0U;
    inline constexpr std::uint32_t k_initialMeshLODTriangleCount = 0U;

    inline constexpr float k_initialMeshLODSimplifyError = 0.0F;
    inline constexpr float k_initialMeshLODMorphError    = 0.0F;

    inline constexpr float k_initialMeshletDataBoundsRadius = 0.0F;

    inline constexpr float k_initialLODSurfaceDeviationScale = 1.0F;
    inline constexpr float k_maxLODSurfaceDeviationScale     = 4.0F;

    // HLSL側(Model.hlsliのk_modelMaxLODCoveredLayerCount)と同じ値にする
    inline constexpr std::uint32_t k_maxLODCoveredLayerCount = 2U;
}