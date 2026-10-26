#pragma once

namespace FWK::Constant
{
    // HLSL側(GPUParticle.hlsli)の構造体のサイズ
    inline constexpr std::size_t k_gpuParticleStructByteSize            = 64ULL;
    inline constexpr std::size_t k_gpuParticleEmitterStructByteSize     = 112ULL;
    inline constexpr std::size_t k_gpuParticleIndirectArgumentsByteSize = 32ULL;

    inline constexpr float         k_initialGPUParticleFloatValue = 0.0F;
    inline constexpr std::uint32_t k_initialGPUParticleUIntValue  = 0U;

    inline constexpr float k_defaultGPUParticleEmitterLifetime     = 1.0F;
    inline constexpr float k_defaultGPUParticleEmitterSize         = 0.1F;
    inline constexpr float k_defaultGPUParticleEmitterGravityScale = 1.0F;
}