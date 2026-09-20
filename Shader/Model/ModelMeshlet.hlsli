#ifndef MODEL_MESHLET_HLSLI
#define MODEL_MESHLET_HLSLI

// normaliaze前に長さ0付近のベクトルを避けるための値
// カメラがconeApexのほぼ同位置にある場合などを安全側に倒す
static const float k_modelMeshletCullingEpsilon     = 0.000001F;
static const float k_modelDisabledMeshletConeCutoff = 1.0F;

static const uint k_modelMAXMeshletVertexCount    = 64U;
static const uint k_modelMAXMeshletPrimitiveCount = 126U;

// uint32_tからuint8_t相当のPrimitiveIndexだけを取り出すためのMask。
// 0xFFU = 下位8bitだけを残す。
static const uint k_modelPackedPrimitiveIndexValueMask = 0xFFU;

static const uint k_modelFirstPackedPrimitiveIndexBitShiftCount  = 0U;
static const uint k_modelSecondPackedPrimitiveIndexBitShiftCount = 8U;
static const uint k_modelThirdPackedPrimitiveIndexBitShiftCount  = 16U;

struct ModelMeshlet
{
    uint vertexOffset;
    uint vertexCount;
    uint triangleOffset;
    uint triangleCount;
};

// ModelのMeshlet1個分のカリング用境界情報
// もし背面法によるアウトラインなどのシェーダーを実装したければ
// バックフェースコーンカリング用のフラグを持たせること
// 現在のBackfaceConeCullingは、WorldMatrixの各軸Scaleが
// 正の非Zero値であることを前提とする。
// 負のScaleによるTriangleの表裏反転は考慮しない。
struct ModelMeshletBounds
{
    // Meshletを囲むBoundingSphereの中心。
    // meshopt_computeMeshletBounds由来なので、基本的にLocal空間
    float3 center;
    
    // BoundingSphereの半径
    // Local空間の半径なので、World空間ではg_worldMaxScaleを掛ける
    float radius;
    
    // BackfaceConeCulling用のコーン頂点位置
    // BackfaceConeCullingはカメラから見てMeshletの三角形が
    // 全部裏向きなら描画しないというカリング
    // その裏向き判定に使う基準位置
    float3 coneApex;
    
    // BackfaceConeCulling用のしきい値
    // coneAxisとconeApexからカメラへ向かう方向の内積と比較して使う
    float coneCutoff;
    
    // BackfaceConeCulling用のコーン方向
    // Meshlet内の三角形群がおおよそどちらを向いているかを表す方向ベクトル
    float3 coneAxis;
    float  padding;
};

// Pack済みuintから、Triangleを構成する三個の
// Meshlet内VertexIndexを取り出す
uint3 DecodeModelPackedPrimitiveIndex(const uint a_packedPrimitiveIndex)
{
    const uint l_firstPrimitiveIndex  = (a_packedPrimitiveIndex >> k_modelFirstPackedPrimitiveIndexBitShiftCount)  & k_modelPackedPrimitiveIndexValueMask;
    const uint l_secondPrimitiveIndex = (a_packedPrimitiveIndex >> k_modelSecondPackedPrimitiveIndexBitShiftCount) & k_modelPackedPrimitiveIndexValueMask;
    const uint l_thirdPrimitiveIndex  = (a_packedPrimitiveIndex >> k_modelThirdPackedPrimitiveIndexBitShiftCount)  & k_modelPackedPrimitiveIndexValueMask;
    
    return uint3
    (
        l_firstPrimitiveIndex,
        l_secondPrimitiveIndex,
        l_thirdPrimitiveIndex
    );
}

#endif // MODEL_MESHLET_HLSLI