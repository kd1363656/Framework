#include "../ModelShader.hlsli"

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
    
    row_major matrix g_worldInverseTransposeMatrix;
    
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

uint FetchStaticModelPackedPrimitiveIndex(const uint a_primitiveByteIndex)
{
    StructuredBuffer<uint> l_packedPrimitiveIndexBuffer = ResourceDescriptorHeap[g_primitiveIndexBufferSRVDescriptorIndex];
    
    // 4個のuint8_tを1このuint32_tにPackしているため、
    // byteIndexを4で割るとPack済みuint32_t配列のIndexになる
    // 例 : 6番目を読み取りたいなら6/4でl_packedPrimitiveIndexで1とあらわすことができる
    const uint l_packedPrimitiveIndex = a_primitiveByteIndex / k_modelPrimitiveIndexPerUnit;
    
    // 4で割った余りを求めることで、余りが、uint32_tの中の何byte目かを読むかになる
    const uint l_byteIndex   = a_primitiveByteIndex % k_modelPrimitiveIndexPerUnit;
    const uint l_packedValue = l_packedPrimitiveIndexBuffer[l_packedPrimitiveIndex];
    const uint l_shiftBit    = l_byteIndex * k_modelPackedPrimitiveIndexBitCount;
    
    return (l_packedValue >> l_shiftBit) & k_modelPackedPrimitiveIndexValueMask;
}

// StaticModelのLocal座標をWorld座標へ変換する
float4 TransformStaticModelLocalPositionToViewProjection(const float3 a_localPosition)
{
    const float4 l_localPosition          = float4(a_localPosition, k_modelPositionElementW);
    const float4 l_worldPosition          = mul   (l_localPosition, g_worldMatrix);
    const float4 l_viewProjectionPosition = mul   (l_worldPosition, g_viewProjectionMatrix);
    
    return l_viewProjectionPosition;
}