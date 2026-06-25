#include "../Model.hlsli"

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

SamplerState g_textureSampler : register(s0);

// 三角形1個分のPrimitiveIndexをuint3で取得する
// 3個Pack方式では、uint32_t1個に三角形1個分のPrimitiveIndexを入れている
// bit配置
// 0  : 1個目のPrimitiveIndex
// 8  : 2個目のPrimitiveIndex
// 16 : 3個目のPrimitiveIndex
// 24 : 未使用
// 戻り値のuint3は、元VertexBufferのIndexではなく、
// MeshShaderが出力したa_vertexListの何番目を使うかを表す。
uint3 FetchStaticModelPackedPrimitiveIndex(const uint a_packedPrimitiveIndex)
{
    StructuredBuffer<uint> l_packedPrimitiveIndexBuffer = ResourceDescriptorHeap[g_primitiveIndexBufferSRVDescriptorIndex];
    
    // 3個数Pack方式では、uint23_t1個が三角形1個分のPrimitiveIndexを持つ。
    // そのため、a_packedPrimitiveIndexはPack済みPrimitiveIndexBuffer上のIndex。
    const uint l_packedValue = l_packedPrimitiveIndexBuffer[a_packedPrimitiveIndex];
    
    return uint3
    (
        (l_packedValue >> k_modelFirstPackedPrimitiveIndexShiftBit)  & k_modelPackedPrimitiveIndexValueMask,
        (l_packedValue >> k_modelSecondPackedPrimitiveIndexShiftBit) & k_modelPackedPrimitiveIndexValueMask,
        (l_packedValue >> k_modelThirdPackedPrimitiveIndexShiftBit)  & k_modelPackedPrimitiveIndexValueMask
    );
}

// StaticModelのLocal座標をWorld座標へ変換する
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