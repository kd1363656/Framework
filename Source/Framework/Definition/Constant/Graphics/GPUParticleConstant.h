#pragma once

namespace FWK::Constant
{
    // 粒子の最大数(全スロットをGPU上に持ち続けるため、この数 × 64byteのメモリを作成時に確保する)
    inline constexpr std::size_t k_gpuParticleMaxCount = 1048576ULL;

    // 1フレームに送れる発生源(エミッター)の最大数と、1フレームに出せる粒子の最大数
    inline constexpr std::size_t   k_gpuParticleMaxEmitterCount      = 1024ULL;
    inline constexpr std::uint32_t k_gpuParticleMaxEmitCountPerFrame = static_cast<std::uint32_t>(k_gpuParticleMaxCount);

    // CounterBufferの要素番号(HLSL側(GPUParticle.hlsli)のk_gpuParticleDeadCounterIndexと必ず同じ値にする)
    // 生存数は読み側と書き側の2つあり、AliveListBufferの番号(0、1)と同じ並びで置く
    inline constexpr std::uint32_t k_gpuParticleDeadCounterIndex    = 0U;
    inline constexpr std::uint32_t k_gpuParticleAliveCounterIndex0  = 1U;
    inline constexpr std::uint32_t k_gpuParticleAliveCounterIndex1  = 2U;
    inline constexpr std::size_t   k_gpuParticleCounterElementCount = 3ULL;

    inline constexpr std::size_t k_gpuParticleAliveListBufferCount = 2ULL;

    inline constexpr std::array<std::uint32_t, k_gpuParticleAliveListBufferCount> k_gpuParticleAliveCounterIndexList =
    {
        k_gpuParticleAliveCounterIndex0,
        k_gpuParticleAliveCounterIndex1,
    };

    // HLSL側(GPUParticle.hlsli)の1Groupあたりのスレッド数と必ず同じ値にする
    inline constexpr std::uint32_t k_gpuParticleInitializeThreadCount = 256U;
    inline constexpr std::uint32_t k_gpuParticleEmitThreadCount       = 256U;
    inline constexpr std::uint32_t k_gpuParticleSimulateThreadCount   = 256U;
    inline constexpr std::uint32_t k_gpuParticleMeshShaderThreadCount = 64U;

    // 1フレーム分の定数バッファの枡(256byteごと)
    // 先頭からComputeShader用(共通、SimulateのGroup数書き込み用、描画のGroup数書き込み用)、その後ろに描画用が並ぶ
    // (描画は別のカメラからの追加の描画でも呼ばれるため、1フレームに複数回書けるように枡を複数用意する)
    inline constexpr std::size_t k_gpuParticleConstantBufferSlotStride                 = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
    inline constexpr std::size_t k_gpuParticleCommonConstantBufferSlotIndex            = 0ULL;
    inline constexpr std::size_t k_gpuParticleSimulateArgumentsConstantBufferSlotIndex = 1ULL;
    inline constexpr std::size_t k_gpuParticleDrawArgumentsConstantBufferSlotIndex     = 2ULL;
    inline constexpr std::size_t k_gpuParticleComputeConstantBufferSlotCount           = 3ULL;
    inline constexpr std::size_t k_gpuParticleDrawConstantBufferSlotCount              = 8ULL;
    inline constexpr std::size_t k_gpuParticleConstantBufferSlotCount                  = k_gpuParticleComputeConstantBufferSlotCount + k_gpuParticleDrawConstantBufferSlotCount;

    // IndirectArgumentsBufferは1要素だけで、Simulate用のDispatch引数と描画用のDispatchMesh引数を16byteずつ置く
    inline constexpr std::size_t k_gpuParticleIndirectArgumentsElementCount = 1ULL;
    inline constexpr UINT        k_gpuParticleIndirectArgumentsStride       = 16U;
    inline constexpr UINT64      k_gpuParticleSimulateArgumentsByteOffset   = 0ULL;
    inline constexpr UINT64      k_gpuParticleDrawArgumentsByteOffset       = static_cast<UINT64>(k_gpuParticleIndirectArgumentsStride);

    inline constexpr std::uint32_t k_gpuParticleInitialEmitCount = 0U;

    inline constexpr TypeAlias::Math::Vector3 k_gpuParticleDefaultGravity = { 0.0F, -9.8F, 0.0F };
}