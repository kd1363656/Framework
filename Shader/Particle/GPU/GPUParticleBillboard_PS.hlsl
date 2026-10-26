#include "GPUParticleBillboard.hlsli"

// GPUパーティクルのピクセルシェーダー
//
// 板の中心からの距離で「中心が濃く、端で消える丸い粒」を作る(テクスチャ無しで描ける)
// 出力は事前乗算アルファ(色に不透明度を掛けた物)で、PipelineStateのブレンドはONE / INV_SRC_ALPHAにしている
// この組み合わせだと、色のaが1なら普通の半透明、0なら加算(光のように明るくなる)になり、aの値で混ぜ方を連続的に選べる

// uv(0～1)を中心が0、端が±1になるように変換するときの係数
static const float k_uvToCenteredScale  = 2.0F;
static const float k_uvToCenteredOffset = 1.0F;

// 端で0になる値の最大(中心)
static const float k_maxSoftness = 1.0F;

float4 main(const GPUParticleMSOutput a_input) : SV_Target0
{
    const float l_distance = length  (a_input.uv * k_uvToCenteredScale - k_uvToCenteredOffset);
    const float l_softness = saturate(k_maxSoftness - l_distance);

    // 2乗にすると中心に濃さが集まり、端がなだらかに消える
    const float l_coverage = l_softness * l_softness;

    return float4(a_input.color.rgb * l_coverage, a_input.color.a * l_coverage);
}