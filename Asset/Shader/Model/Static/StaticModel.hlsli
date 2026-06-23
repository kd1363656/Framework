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

cbuffer CBLightPass : register(b2)
{
    float3 g_directionalLightDirection;
    float  g_directionalLightIntensity;
    
    float3 g_directionalLightColor;
    float  g_lightPassPadding;
    
    float3 g_ambientLightColor;
    float  g_ambientLightIntensity;
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

// StaticMoidelのLocal座標をWorld座標へ変換する
// PBRではライト方向やカメラ方向をWorld空間で計算するため、worldPositionが必要
float3 TransformStaticModelLocalPositionToWorld(const float3 a_localPosition)
{
    const float4 l_localPosition = float4(a_localPosition, k_modelPositionElementW);
    const float4 l_worldPosition = mul   (l_localPosition, g_worldMatrix);

    return l_worldPosition.xyz;
}

// StaticModelのLocal法線をWorld空間へ変換する
// 法線は位置ではなく方向なのでw = 0
// 非均一スケールでも法線方向が壊れにくいようにWorldInverseTransposeMatrixを使う。
float3 TransformStaticModelLocalNormalToWorld(const float3 a_localNormal)
{
    const float4 l_localNormal = float4(a_localNormal, k_modelDirectionElementW);
    const float4 l_worldNormal = mul   (l_localNormal, g_worldInverseTransposeMatrix);
    
    return normalize(l_worldNormal.xyz);
}

// StaticModelのLocalTangentをWorld空間へ変換する
// Tangentは方向なのでw = 0。
// tangent.wはNormalMap用の向き補正なので維持する
float4 TransformStaticModelLocalTangentToWorld(const float4 a_localTangent)
{
    const float4 l_localTangent = float4(a_localTangent.xyz, k_modelDirectionElementW);
    const float4 l_worldTangent = mul   (l_localTangent,     g_worldMatrix);

    return float4(normalize(l_worldTangent.xyz), a_localTangent.w);
}