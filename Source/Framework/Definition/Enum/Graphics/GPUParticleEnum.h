#pragma once

namespace FWK::Enum
{
    // GPUパーティクルのIndirectArgumentsBuild_CSが、どちらのGroup数を書くか
    // ※ HLSL側(GPUParticle.hlsli)のk_gpuParticleIndirectArgumentsBuildTargetXXXと同じ値にする
    enum class GPUParticleIndirectArgumentsBuildTarget : std::uint32_t
    {
        // 読み側の生存数からSimulate_CSのDispatchのGroup数を書く(書き側の生存数も0に戻す)
        Simulate,

        // 書き側の生存数から描画のDispatchMeshのGroup数を書く
        Draw,
    };
}