#ifndef MODEL_STANDARD_HLSLI
#define MODEL_STANDARD_HLSLI
#include "../Model.hlsli"
#include "../../Camera/CameraPass.hlsli"

SamplerState g_textureSampler : register(s0);

// ModelのLocal法線をWorld空間へ変換する
// 法線は位置ではなく方向なのでw = 0
// 非均一スケールでも法線方向が壊れにくいようにWorldInverseTransposeMatrixを使う
float3 TransformModelLocalNormalToWorld(const ModelObjectData a_object, const float3 a_localNormal)
{
    const float4 l_localNormal = float4(a_localNormal, k_modelDirectionElementW);
    const float4 l_worldNormal = mul   (l_localNormal, a_object.worldInverseTransposeMatrix);

    return normalize(l_worldNormal.xyz);
}

// ModelのLocalTangentをWorld空間へ変換する
// Tangentは方向なのでw = 0
// tangent.wはNormalMap用の向き補正なので維持する
float4 TransformModelLocalTangentToWorld(const ModelObjectData a_object, const float4 a_localTangent)
{
    const float4 l_localTangent = float4(a_localTangent.xyz, k_modelDirectionElementW);
    const float4 l_worldTangent = mul   (l_localTangent,     a_object.worldMatrix);

    return float4(normalize(l_worldTangent.xyz), a_localTangent.w * a_object.worldOrientationSign);
}

// ベースカラーのテクスチャを読み、色を掛ける
// マテリアルのテーブルを直接読まず、番号と色を引数で受け取る(規約 19-8)
// こうすると、Lit / UnLit / トゥーンなど、マテリアルの種類が違っても同じ関数を使える
float4 FetchModelBaseColor(const uint a_baseColorTextureSRVDescriptorIndex, const float4 a_baseColor, const float2 a_uv)
{
    Texture2D<float4> l_baseColorTexture = ResourceDescriptorHeap[a_baseColorTextureSRVDescriptorIndex];

    const float4 l_baseColorSample = l_baseColorTexture.Sample(g_textureSampler, a_uv);

    return l_baseColorSample * a_baseColor;
}

#endif // MODEL_STANDARD_HLSLI