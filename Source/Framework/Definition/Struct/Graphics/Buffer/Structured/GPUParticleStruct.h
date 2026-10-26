#pragma once

namespace FWK::Struct
{
    // GPUパーティクルで使用する構造体
    // ※ HLSL側(Shader/Particle/GPU/GPUParticle.hlsli)と必ず同じ並びにする

    // 粒子1つ分(GPUだけが読み書きし、CPUは中身を作らない。Bufferの要素の型として使う)
    // m_colorBeginLow / High、m_colorEndLow / High : RGBAを16bitの浮動小数(half)にして2つのuintへ詰めた色
    // m_sizeBeginEnd                              : 生まれたときと寿命が来たときの大きさ(halfを2つ詰めた物)
    // m_dragGravityScale                          : 空気抵抗と重力の効き具合(halfを2つ詰めた物)
    struct SBGPUParticle final
    {
        TypeAlias::Math::Vector3 m_position = TypeAlias::Math::Vector3::Zero;
        float                    m_age      = Constant::k_initialGPUParticleFloatValue;

        TypeAlias::Math::Vector3 m_velocity = TypeAlias::Math::Vector3::Zero;
        float                    m_lifetime = Constant::k_initialGPUParticleFloatValue;

        std::uint32_t m_colorBeginLow  = Constant::k_initialGPUParticleUIntValue;
        std::uint32_t m_colorBeginHigh = Constant::k_initialGPUParticleUIntValue;
        std::uint32_t m_colorEndLow    = Constant::k_initialGPUParticleUIntValue;
        std::uint32_t m_colorEndHigh   = Constant::k_initialGPUParticleUIntValue;

        std::uint32_t m_sizeBeginEnd     = Constant::k_initialGPUParticleUIntValue;
        float         m_rotation         = Constant::k_initialGPUParticleFloatValue;
        float         m_angularVelocity  = Constant::k_initialGPUParticleFloatValue;
        std::uint32_t m_dragGravityScale = Constant::k_initialGPUParticleUIntValue;
    };

    static_assert(sizeof(SBGPUParticle) == Constant::k_gpuParticleStructByteSize, "SBGPUParticleのサイズがHLSL側と一致しません。");

    // 発生源(エミッター)1つ分。CPUが毎フレーム「このフレームに出す粒子の設定」としてGPUへ送る
    // m_emitOffset : 今フレームに出す全粒子を1列に並べたとき、この発生源の粒子が何番目から始まるか(GPUParticlePerObjectDrawRequestが決める)
    // m_emitCount  : この発生源が今フレームに出す数
    struct SBGPUParticleEmitter final
    {
        TypeAlias::Math::Vector3 m_position   = TypeAlias::Math::Vector3::Zero;
        std::uint32_t            m_emitOffset = Constant::k_initialGPUParticleUIntValue;

        TypeAlias::Math::Vector3 m_positionRandomRange = TypeAlias::Math::Vector3::Zero;
        std::uint32_t            m_emitCount           = Constant::k_initialGPUParticleUIntValue;

        TypeAlias::Math::Vector3 m_velocity            = TypeAlias::Math::Vector3::Zero;
        float                    m_velocityRandomRange = Constant::k_initialGPUParticleFloatValue;

        TypeAlias::Math::Color m_colorBegin = Constant::k_whiteColor;
        TypeAlias::Math::Color m_colorEnd   = Constant::k_whiteColor;

        float m_lifetimeMIN = Constant::k_defaultGPUParticleEmitterLifetime;
        float m_lifetimeMAX = Constant::k_defaultGPUParticleEmitterLifetime;
        float m_sizeBegin   = Constant::k_defaultGPUParticleEmitterSize;
        float m_sizeEnd     = Constant::k_defaultGPUParticleEmitterSize;

        float m_rotationRandomRange        = Constant::k_initialGPUParticleFloatValue;
        float m_angularVelocityRandomRange = Constant::k_initialGPUParticleFloatValue;
        float m_drag                       = Constant::k_initialGPUParticleFloatValue;
        float m_gravityScale               = Constant::k_defaultGPUParticleEmitterGravityScale;
    };

    static_assert(sizeof(SBGPUParticleEmitter) == Constant::k_gpuParticleEmitterStructByteSize, "SBGPUParticleEmitterのサイズがHLSL側と一致しません。");

    // ExecuteIndirectが読む引数(GPUのIndirectArgumentsBuild_CSが書く)
    // Simulate用のDispatchの引数と、描画用のDispatchMeshの引数を16byteずつの区切りに置く
    struct GPUParticleIndirectArguments final
    {
        D3D12_DISPATCH_ARGUMENTS m_simulateDispatchArguments = {};
        std::uint32_t            m_padding0                  = Constant::k_initialGPUParticleUIntValue;

        D3D12_DISPATCH_MESH_ARGUMENTS m_drawDispatchMeshArguments = {};
        std::uint32_t                 m_padding1                  = Constant::k_initialGPUParticleUIntValue;
    };

    static_assert(sizeof(GPUParticleIndirectArguments) == Constant::k_gpuParticleIndirectArgumentsByteSize, "GPUParticleIndirectArgumentsのサイズがHLSL側と一致しません。");
}