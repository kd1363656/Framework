#ifndef GPU_PARTICLE_BILLBOARD_HLSLI
#define GPU_PARTICLE_BILLBOARD_HLSLI
#include "GPUParticle.hlsli"
#include "../../Camera/CameraPass.hlsli"

// メッシュシェーダーからピクセルシェーダーへ渡す頂点
// uv : 板の中での位置(0～1)。ピクセルシェーダーが中心からの距離で丸い形を作るのに使う
struct GPUParticleMSOutput
{
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
    float4 color    : COLOR0;
};

#endif // GPU_PARTICLE_BILLBOARD_HLSLI