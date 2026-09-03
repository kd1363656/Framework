#ifndef MODEL_HLSLI
#define MODEL_HLSLI
#include "ModelMeshlet.hlsli"

static const float k_modelPositionElementW  = 1.0F;
static const float k_modelDirectionElementW = 0.0F;

static const float k_modelNormalWorldOrientationSign = 1.0F;

// WorldMatrixのdeterminantが負となり
// TriangleのWindingが反転してる状態
static const float k_modelMirroredWorldOrientationSign = -1.0F;

// Frustumの側面Planeに対するSphere半径補正で使う
// sqrt(1.0 + tanFOV * tanFOV)の1.0部分。
static const float k_modelFrustumPlaneNormalBaseLength = 1.0F;

static const uint k_modelMeshShaderThreadCountX = 32U;
static const uint k_modelMeshShaderThreadCountY = 1U;
static const uint k_modelMeshShaderThreadCountZ = 1U;

// 1個のAmplificationShaderGroupで
// 32個のMeshletを並列にカリングする
// C++側のConstant::k_meshletCountPerAmplificationShaderGroupと必ず同じ値にする
static const uint k_modelAmplificationShaderThreadCountX = 32U;
static const uint k_modelAmplificationShaderThreadCountY = 1U;
static const uint k_modelAmplificationShaderThreadCountZ = 1U;

static const uint k_modelAmplificationDispatchMeshGroupCountY = 1U;
static const uint k_modelAmplificationDispatchMeshGroupCountZ = 1U;

static const uint k_modelAmplificationInitialVisibleMeshletCount = 0U;

static const uint k_modelAmplificationVisibleMeshletCountIncrement = 1U;

static const uint k_modelAmplificationLeaderThreadIndex = 0U;

// 1つのAmplificationShaderGroupが可視判定を通過した
// MeshletIndexを子MeshShaderGroupへ渡すPayload
// 配列には可視MeshletIndexだけが先頭から連続して格納される
struct ModelAmplificationPayload
{
    uint meshletIndexList[k_modelAmplificationShaderThreadCountX];
};

cbuffer CBModelPerObject : register(b1)
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
    
    float  g_worldOrientationSign;
    uint   g_meshletCount;
    float2 g_padding;
};

// 三角形1個分のPrimitiveIndexをuint3で取得する
// 3個Pack方式では、uint32_t1個に三角形1個分のPrimitiveIndexを入れている
// また、WorldMatrixのdeterminant画布の場合は、
// WorldTransformによってTriangleのWindingが反転する
// bit配置
// 0  : 1個目のPrimitiveIndex
// 8  : 2個目のPrimitiveIndex
// 16 : 3個目のPrimitiveIndex
// 24 : 未使用
// 戻り値のuint3は、元VertexBufferのIndexではなく、
// MeshShaderが出力したa_vertexListの何番目を使うかを表す。
uint3 FetchModelPackedPrimitiveIndex(const uint a_packedPrimitiveIndex)
{
    StructuredBuffer<uint> l_packedPrimitiveIndexBuffer = ResourceDescriptorHeap[g_primitiveIndexBufferSRVDescriptorIndex];
    
    // uint一個に三角形一個分の
    // 三つのPrimitiveIndexがPackされている
    const uint  l_packedValue    = l_packedPrimitiveIndexBuffer[a_packedPrimitiveIndex];
    const uint3 l_primitiveIndex = DecodeModelPackedPrimitiveIndex(l_packedValue);
    
    // determinantが負のWorldMatrixでは
    // (0, 1, 2)のTriangleを、(0, 2, 1)
    // へ変更することでWindingを元に戻す
    if (g_worldOrientationSign == k_modelMirroredWorldOrientationSign) { return uint3(l_primitiveIndex.x, l_primitiveIndex.z, l_primitiveIndex.y); }
    
    return l_primitiveIndex;
}

// StaticModelのLocal座標をWorld座標へ変換する
// PBRではライト方向やカメラ方向をWorld空間で計算するため、worldPositionが必要
float3 TransformModelLocalPositionToWorld(const float3 a_localPosition)
{
    const float4 l_localPosition = float4(a_localPosition, k_modelPositionElementW);
    const float4 l_worldPosition = mul   (l_localPosition, g_worldMatrix);

    return l_worldPosition.xyz;
}

#endif // MODEL_HLSLI