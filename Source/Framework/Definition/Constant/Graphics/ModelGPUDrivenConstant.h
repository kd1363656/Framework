#pragma once

namespace FWK::Constant
{
    // カリング用ComputeShaderの1Groupあたりのスレッド数
    // HLSL側(ModelGPUDrivenCulling_CS.hlsl)のk_modelGPUDrivenCullingThreadCountXと必ず同じ値にする
    inline constexpr std::uint32_t k_modelGPUDrivenCullingThreadCount = 64U;

    inline constexpr std::uint32_t k_modelGPUDrivenCounterElementCount = 3U;

    // 各Bufferに入れられる最大数(表を作る側は、この数を超えないようにする)
    inline constexpr std::size_t k_modelGPUDrivenMaxInstanceCount = 4096ULL;
    inline constexpr std::size_t k_modelGPUDrivenMaxMeshInfoCount = 1024ULL;
    inline constexpr std::size_t k_modelGPUDrivenMaxLODCount      = 8192ULL;

    inline constexpr std::size_t k_modelGPUDrivenMaxMorphWeightCount = 65536ULL;
}