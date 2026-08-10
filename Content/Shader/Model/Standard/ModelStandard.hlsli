#ifndef MODEL_STANDARD_HLSLI
#define MODEL_STANDARD_HLSLI
#include "../Model.hlsli"
#include "../../Camera/CameraPass.hlsli"

SamplerState g_textureSampler : register(s0);

// StaticModelのLocal法線をWorld空間へ変換する
// 法線は位置ではなく方向なのでw = 0
// 非均一スケールでも法線方向が壊れにくいようにWorldInverseTransposeMatrixを使う。
float3 TransformModelLocalNormalToWorld(const float3 a_localNormal)
{
    const float4 l_localNormal = float4(a_localNormal, k_modelDirectionElementW);
    const float4 l_worldNormal = mul   (l_localNormal, g_worldInverseTransposeMatrix);
    
    return normalize(l_worldNormal.xyz);
}

// StaticModelのLocalTangentをWorld空間へ変換する
// Tangentは方向なのでw = 0。
// tangent.wはNormalMap用の向き補正なので維持する
float4 TransformModelLocalTangentToWorld(const float4 a_localTangent)
{
    const float4 l_localTangent = float4(a_localTangent.xyz, k_modelDirectionElementW);
    const float4 l_worldTangent = mul   (l_localTangent,     g_worldMatrix);

    return float4(normalize(l_worldTangent.xyz), a_localTangent.w);
}

float4 FetchModelBaseColor(const float2 a_uv)
{
    Texture2D<float4> l_baseColorTexture = ResourceDescriptorHeap[g_baseColorTextureSRVDescriptorIndex];
    
    const float4 l_baseColorSample = l_baseColorTexture.Sample(g_textureSampler, a_uv);
    
    return l_baseColorSample * g_baseColorFactor;
}

#endif // MODEL_STANDARD_HLSLI