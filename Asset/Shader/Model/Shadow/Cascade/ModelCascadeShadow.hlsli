#ifndef MODEL_CASCADE_SHADOW_HLSLI
#define MODEL_CASCADE_SHADOW_HLSLI

// 現在描画しているCascadeのLightViewProjection行列。
// StaticModelとSkeletalAnimationModelのShadow描画で
// 同じConstantBufferを使用する
cbuffer CBCascadeShadowPass : register(b0)
{
    row_major matrix g_cascadeViewProjectionMatrix;
    row_major matrix g_cascadeViewMatrix;
    
    float3 g_cascadeOrthographicsMIN;
    float  g_cascadeOrthographicMINPadding;
    
    float3 g_cascadeOrthographicMAX;
    float  g_cascadeOrthographicMAXPadding;
    
    float3 g_directionalLightDirection;
    float  g_directionalLightDirectionPadding;
}

// ShadowMapにはDepthだけを書き込むため、
// MeshShaderからはClip座標だけを出力する
struct MSOutputCascadeShadow
{
    float4 position : SV_Position;
};

#endif // MODEL_CASCADE_SHADOW_HLSLI