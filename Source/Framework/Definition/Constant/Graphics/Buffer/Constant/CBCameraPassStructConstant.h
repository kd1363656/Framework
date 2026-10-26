#pragma once

namespace FWK::Constant
{
    static constexpr float k_cameraDefaultNearClip        = 0.01F;
    static constexpr float k_cameraDefaultFarClip         = 2000.0F;
    static constexpr float k_cameraDefaultTangentHalfFOVX = 0.0F;
    static constexpr float k_cameraDefaultTangentHalfFOVY = 0.0F;

    // CBCameraPassの大きさ(HLSL側の定数バッファと合わせるための確認用)
    inline constexpr std::size_t k_cbCameraPassByteSize = 320ULL;

    // m_isCullingDebugVisualizeの値(HLSL側はk_trueValue / k_modelFalseValueで判定する)
    inline constexpr std::uint32_t k_cameraPassCullingDebugVisualizeOff = 0U;
    inline constexpr std::uint32_t k_cameraPassCullingDebugVisualizeOn  = 1U;
}
