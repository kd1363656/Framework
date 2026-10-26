#pragma once

namespace FWK::Constant
{
    inline constexpr float k_initialGPUDrivenInstanceWorldMaxScale = 1.0F;

    inline constexpr std::uint32_t k_initialGPUDrivenInstanceAnimationFrameIndex = 0U;
    inline constexpr std::uint32_t k_invalidGPUDrivenMorphWeightOffset           = std::numeric_limits<std::uint32_t>::max();

    inline constexpr float k_initialGPUDrivenMeshInfoBoundsRadius = 0.0F;

    inline constexpr std::uint32_t k_initialGPUDrivenMeshInfoCount = 0U;

    inline constexpr std::uint32_t k_initialGPUDrivenMeshInfoPadding = 0U;

    inline constexpr float k_initialGPUDrivenMeshInfoLODSurfaceDeviationScale = 1.0F;

    inline constexpr std::uint32_t k_initialGPUDrivenSkinningInfoValue = 0U;

    inline constexpr std::uint32_t k_initialGPUDrivenBatchValue = 0U;

    inline constexpr std::size_t k_initialGPUDrivenFrameTableTotalPairCount = 0ULL;

    inline constexpr std::uint64_t k_initialGPUDrivenFrameTableVersion = 0ULL;

    inline constexpr std::size_t k_modelGPUDrivenDrawRecordByteSize = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;

    inline constexpr std::size_t k_modelGPUDrivenIndirectCommandByteSize = 24ULL;

    inline constexpr std::uint32_t k_initialGPUDrivenIndirectCommandPadding = 0U;

    // HLSL側の定数バッファのサイズ
    inline constexpr std::size_t k_cbModelGPUDrivenCullingPassByteSize = 304ULL;

    inline constexpr std::size_t k_maxGPUDrivenFrustumPlaneCount = 6ULL;

    inline constexpr float k_initialGPUDrivenCullingPassFloatValue = 0.0F;

    inline constexpr std::uint32_t k_initialGPUDrivenCullingPassUIntValue = 0U;

    inline constexpr std::uint32_t k_initialGPUDrivenOcclusionCandidateIndex = 0U;

    inline constexpr std::size_t k_initialGPUDrivenCandidateSourceViewIndex = 0ULL;

    inline constexpr float k_initialGPUDrivenOrthographicTexelPerWorldUnit = 0.0F;

    inline constexpr std::uint32_t k_initialGPUDrivenFrustumPlaneCount = 0U;
}