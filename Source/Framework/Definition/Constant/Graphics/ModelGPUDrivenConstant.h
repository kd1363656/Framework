#pragma once

namespace FWK::Constant
{
    // カリング用ComputeShaderの1Groupあたりのスレッド数
    // HLSL側(ModelGPUDrivenCulling_CS.hlsl)のk_modelGPUDrivenCullingThreadCountXと必ず同じ値にする
    inline constexpr std::uint32_t k_modelGPUDrivenCullingThreadCount = 64U;

    // GPUドリブン描画のカウンター(描画数、三角形数、オクルージョンカリングの候補数)の要素数
    inline constexpr std::uint32_t k_modelGPUDrivenCounterElementCount = 3U;
}