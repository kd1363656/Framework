#include "../../ModelShader.hlsli"

struct StaticModelVertex
{
    float3 position;
    float3 normal;
    float4 tangent;
    float2 uv;
};

cbuffer CBStaticModelPerObject : register(b1)
{
    row_major matrix g_worldMatrix;
    
    float4 g_baseColorFactor;
    
    float g_roughnessFactor;
    float g_metallicFactor;
    uint  g_baseColorTextureSRVDescriptorIndex;
    uint  g_normalTextureSRVDescriptorIndex;
    
    uint g_metallicTextureSRVDescriptorIndex;
    uint g_roughnessTextureSRVDescriptorIndex;
    uint g_vertexBufferSRVDescriptorIndex;
    uint g_meshletBufferSRVDescriptorIndex;
    
    uint  g_uniqueVertexIndexBufferSRVDescriptorIndex;
    uint  g_primitiveIndexBufferSRVDescriptorIndex;
    uint  g_meshletBoundsBufferSRVDescriptorIndex;
    float g_worldMaxScale;
};

static const uint k_staticModelTriangleVertexCount = 3U;

SamplerState g_baseColorSampler : register(s0);

// StaticModelのLocal座標をWorld座標へ変換する
float3 TransformStaticModelLocalPositionToWorld(const float3 a_localPosition)
{
    return mul(float4(a_localPosition, k_modelPositionVectorElementW), g_worldMatrix).xyz;
}

// StaticModelのLocal方向ベクトルをWorld方向ベクトルへ変換する
// 方向ベクトルなのでwは0.0にする、これにより、移動成分の影響を受けない
float3 TransformStaticModelLocalDirectionToWorld(const float3 a_localDirection)
{
    return normalize(mul(float4(a_localDirection, k_modelDirectionVectorElementW), g_worldMatrix).xyz);
}