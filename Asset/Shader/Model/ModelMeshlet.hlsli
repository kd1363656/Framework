static const uint k_modelMaxMeshletVertexCount    = 64U;
static const uint k_modelMaxMeshletPrimitiveCount = 126U;

// normaliaze前に長さ0付近のベクトルを避けるための値。
// カメラがconeApexのほぼ同位置にある場合などを安全側に倒す。
static const float k_modelMeshletCullingEpsilon = 0.000001F;

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