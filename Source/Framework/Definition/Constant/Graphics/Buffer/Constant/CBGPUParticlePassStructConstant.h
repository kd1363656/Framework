#pragma once

namespace FWK::Constant
{
    // HLSL側(GPUParticle.hlsli)の定数バッファのサイズ
    inline constexpr std::size_t k_cbGPUParticlePassByteSize = 96ULL;

    inline constexpr std::uint32_t k_initialCBGPUParticlePassUIntValue  = 0U;
    inline constexpr float         k_initialCBGPUParticlePassFloatValue = 0.0F;
}