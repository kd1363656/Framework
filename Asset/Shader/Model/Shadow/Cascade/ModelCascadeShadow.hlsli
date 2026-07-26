#ifndef MODEL_CASCADE_SHADOW_HLSLI
#define MODEL_CASCADE_SHADOW_HLSLI

// 現在描画しているCascadeのLightViewProjection行列。
// StaticModelとSkeletalAnimationModelのShadow描画で
// 同じConstantBufferを使用する
cbuffer CBCascadeShadowPass : register(b0)
{
    row_major matrix g_cascadeViewProjectionMatrix;
}

// ShadowMapにはDepthだけを書き込むため、
// MeshShaderからはClip座標だけを出力する
struct MSOutputCascadeShadow
{
    float4 position : SV_Position;
};

#endif // MODEL_CASCADE_SHADOW_HLSLI