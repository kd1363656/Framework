#include "../../ModelShader.hlsli"

struct StaticModelVertex
{
    float3 position;
    float3 normal;
    float4 tangent;
    float2 uv;
};

static const uint k_staticModelTriangleVertexCount = 3U;

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

SamplerState g_baseColorSampler : register(s0);

// StaticModelのLocal座標をWorld座標へ変換する
float4 TransformStaticModelLocalPositionToViewProjection(const float3 a_localPosition)
{
    const float4 l_localPosition          = float4(a_localPosition, k_modelPositionElementW);
    const float4 l_worldPosition          = mul   (l_localPosition, g_worldMatrix);
    const float4 l_viewProjectionPosition = mul   (l_worldPosition, g_viewProjectionMatrix);
    
    return l_viewProjectionPosition;
}