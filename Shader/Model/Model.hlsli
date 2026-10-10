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
// sqrt(1.0 + tanFOV * tanFOV)の1.0部分
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

// 1回の描画(メッシュ1つ)ごとに、C++がルート定数で送る番号
// C++側のStruct::RCModelDrawItemと同じ並びにする
// g_objectIndex   : オブジェクトのテーブルの何番目か(行列など)
// g_meshIndex     : メッシュのテーブルの何番目か(バッファのSRVの番号など)
// g_materialIndex : マテリアルのテーブルの何番目か(影のパスでは使わない)
cbuffer RCModelDrawItem : register(b1)
{
    uint g_objectIndex;
    uint g_meshIndex;
    uint g_materialIndex;
};

// オブジェクトとメッシュのテーブルのSRVの番号
// パスの最初に1回だけ送る
// C++側のStruct::RCModelTableと同じ並びにする
cbuffer RCModelTable : register(b5)
{
    uint g_objectTableSRVDescriptorIndex;
    uint g_meshTableSRVDescriptorIndex;
};

// オブジェクトのテーブルの1要素
// C++側のStruct::ModelObjectGPUDataと同じ並び(136バイト)にする
// StructuredBufferの要素なので、cbufferのような16バイト境界のパディングは入れない
struct ModelObjectData
{
    row_major float4x4 worldMatrix;
    row_major float4x4 worldInverseTransposeMatrix;
    float              worldMAXScale;
    float              worldOrientationSign;
};

// メッシュのテーブルの1要素
// C++側のStruct::ModelMeshGPUDataと同じ並び(24バイト)にする
struct ModelMeshData
{
    uint vertexBufferSRVDescriptorIndex;
    uint meshletBufferSRVDescriptorIndex;
    uint uniqueVertexIndexBufferSRVDescriptorIndex;
    uint primitiveIndexBufferSRVDescriptorIndex;
    uint meshletBoundsBufferSRVDescriptorIndex;
    uint meshletCount;
};

// この描画のオブジェクトの値(行列など)を、オブジェクトのテーブルから読む
ModelObjectData FetchModelObjectData()
{
    return (ModelObjectData)0;
}

// この描画のメッシュの値(バッファのSRVの番号など)を、メッシュのテーブルから読む
ModelMeshData FetchModelMeshData()
{
    return (ModelMeshData)0;
}

// 三角形1個分のPrimitiveIndexをuint3で取得する
// 3個Pack方式では、uint32_t1個に三角形1個分のPrimitiveIndexを入れている
// また、WorldMatrixのdeterminantが負の場合は、
// WorldTransformによってTriangleのWindingが反転する
// bit配置
// 0  : 1個目のPrimitiveIndex
// 8  : 2個目のPrimitiveIndex
// 16 : 3個目のPrimitiveIndex
// 24 : 未使用
// 戻り値のuint3は、元VertexBufferのIndexではなく、
// MeshShaderが出力したa_vertexListの何番目を使うかを表す
uint3 FetchModelPackedPrimitiveIndex(const ModelObjectData a_object, const ModelMeshData a_mesh, const uint a_packedPrimitiveIndex)
{
    return uint3(0U, 0U, 0U);
}

// ModelのLocal座標をWorld座標へ変換する
// PBRではライト方向やカメラ方向をWorld空間で計算するため、worldPositionが必要
float3 TransformModelLocalPositionToWorld(const ModelObjectData a_object, const float3 a_localPosition)
{
    return float3(0.0F, 0.0F, 0.0F);
}

#endif // MODEL_HLSLI