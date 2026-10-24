#pragma once

namespace FWK::Enum
{
    // GPUドリブン描画のカリングの種類
    // ※ HLSL側(ModelGPUDrivenCulling_CS.hlsl)のk_cullingModeXXXと同じ値にする
    enum class ModelGPUDrivenCullingMode : std::uint32_t
    {
        // 視錐台カリングのみ(影のカスケードなど)
        Normal,

        // オクルージョンカリング1回目 : 前のフレームのHi-Zで判定し、隠れていた物は候補として残す
        Early,

        // オクルージョンカリング2回目 : 今のフレームのHi-Zで、1回目の候補だけを判定し直す
        Late,
    };
}