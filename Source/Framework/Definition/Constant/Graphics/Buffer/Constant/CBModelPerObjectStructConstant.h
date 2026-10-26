#pragma once

namespace FWK::Constant
{
    inline constexpr std::uint32_t k_defaultMeshletCount = 0U;

    inline constexpr float k_initialCBModelPerObjectLODCoveredLayerDepthOffset = 0.0F;

    inline constexpr std::uint32_t k_initialCBModelPerObjectSkinningValue = 0U;

    // カリングの可視化用の状態の初期値(0 = 見えている。HLSL側のk_modelCullingDebugStateVisibleと同じ値)
    inline constexpr std::uint32_t k_initialCBModelPerObjectCullingDebugState = 0U;
}