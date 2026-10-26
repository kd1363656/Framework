#pragma once

namespace FWK::Struct
{
    // GPUパーティクルの1フレーム分の設定(全てのComputeShaderとメッシュシェーダーが同じ物を読む)
    // ※ HLSL側(Shader/Particle/GPU/GPUParticle.hlsli)と必ず同じ並びにする
    // m_aliveReadCounterIndex / m_aliveWriteCounterIndex : CounterBufferの何番目が読み側 / 書き側の生存数か(毎フレーム入れ替わる)
    // m_randomSeed                                        : フレームごとに変わる乱数の種
    struct CBGPUParticlePass final
    {
        TypeAlias::DescriptorIndex m_particleBufferUAVDescriptorIndex      = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_particleBufferSRVDescriptorIndex      = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_deadListBufferUAVDescriptorIndex      = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_aliveListReadBufferUAVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;

        TypeAlias::DescriptorIndex m_aliveListWriteBufferUAVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_aliveListWriteBufferSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_counterBufferUAVDescriptorIndex        = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_counterBufferSRVDescriptorIndex        = Graphics::DescriptorHeap::k_invalidDescriptorIndex;

        TypeAlias::DescriptorIndex m_indirectArgumentsBufferUAVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_emitterBufferSRVDescriptorIndex           = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        std::uint32_t              m_emitterCount                              = Constant::k_initialCBGPUParticlePassUIntValue;
        std::uint32_t              m_totalEmitCount                            = Constant::k_initialCBGPUParticlePassUIntValue;

        std::uint32_t                                 m_maxParticleCount             = Constant::k_initialCBGPUParticlePassUIntValue;
        std::uint32_t                                 m_aliveReadCounterIndex        = Constant::k_initialCBGPUParticlePassUIntValue;
        std::uint32_t                                 m_aliveWriteCounterIndex       = Constant::k_initialCBGPUParticlePassUIntValue;
        Enum::GPUParticleIndirectArgumentsBuildTarget m_indirectArgumentsBuildTarget = Enum::GPUParticleIndirectArgumentsBuildTarget::Simulate;

        std::uint32_t m_randomSeed = Constant::k_initialCBGPUParticlePassUIntValue;
        float         m_deltaTime  = Constant::k_initialCBGPUParticlePassFloatValue;
        std::uint32_t m_padding0   = Constant::k_initialCBGPUParticlePassUIntValue;
        std::uint32_t m_padding1   = Constant::k_initialCBGPUParticlePassUIntValue;

        TypeAlias::Math::Vector3 m_gravity  = Constant::k_gpuParticleDefaultGravity;
        float                    m_padding2 = Constant::k_initialCBGPUParticlePassFloatValue;
    };

    static_assert(sizeof(CBGPUParticlePass) == Constant::k_cbGPUParticlePassByteSize, "CBGPUParticlePassのサイズがHLSL側の定数バッファと一致しません。");
}