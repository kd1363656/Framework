static const float k_modelPositionElementW  = 1.0F;
static const float k_modelDirectionElementW = 0.0F;

// normaliaze前に長さ0付近のベクトルを避けるための値。
// カメラがconeApexのほぼ同位置にある場合などを安全側に倒す。
static const float k_modelMeshletCullingEpsilon = 0.000001F;

// Frustumの側面Planeに対するSphere半径補正で使う
// sqrt(1.0 + tanFOV * tanFOV)の1.0部分。
static const float k_modelFrustumPlaneNormalBaseLength = 1.0F;

static const uint k_modelTriangleVertexCount = 3U;

static const uint k_modelMaxMeshletVertexCount    = 64U;
static const uint k_modelMaxMeshletPrimitiveCount = 126U;

static const uint k_modelMeshShaderThreadCountX = 32U;
static const uint k_modelMeshShaderThreadCountY = 1U;
static const uint k_modelMeshShaderThreadCountZ = 1U;

static const uint k_modelAmplificationShaderThreadCountX = 1U;
static const uint k_modelAmplificationShaderThreadCountY = 1U;
static const uint k_modelAmplificationShaderThreadCountZ = 1U;

static const uint k_modelAmplificationDispatchMeshGroupCountX = 1U;
static const uint k_modelAmplificationDispatchMeshGroupCountY = 1U;
static const uint k_modelAmplificationDispatchMeshGroupCountZ = 1U;

static const uint k_modelAmplificationDispatchMeshCulledGroupCountX = 0U;

static const uint k_modelFirstPrimitiveVertexOffset  = 0U;
static const uint k_modelSecondPrimitiveVertexOffset = 1U;
static const uint k_modelThirdPrimitiveVertexOffset  = 2U;

static const uint k_modelPackedPrimitiveIndexBitCount = 8U;

// uint32_tからuint8_t相当のPrimitiveIndexだけを取り出すためのMask。
// 0xFFU = 下位8bitだけを残す。
static const uint k_modelPackedPrimitiveIndexValueMask = 0xFFU;

static const uint k_modelFirstPackedPrimitiveIndexShiftBit  = k_modelFirstPrimitiveVertexOffset  * k_modelPackedPrimitiveIndexBitCount;
static const uint k_modelSecondPackedPrimitiveIndexShiftBit = k_modelSecondPrimitiveVertexOffset * k_modelPackedPrimitiveIndexBitCount;
static const uint k_modelThirdPackedPrimitiveIndexShiftBit  = k_modelThirdPrimitiveVertexOffset  * k_modelPackedPrimitiveIndexBitCount;

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

// AmplificationShaderからMeshShaderへ渡すPayload
// AS1グループ = Meshlet1個なので
// MeshShaderへ渡す情報は描画するMeshletIndexだけでよい
struct ModelAmplificationPayload
{
    uint meshletIndex;
};

cbuffer CBCameraPass : register(b0)
{
    row_major matrix g_viewMatrix;
    row_major matrix g_projectionMatrix;
    row_major matrix g_viewProjectionMatrix;
    
    float g_nearClip;
    float g_farClip;
    float g_tanHalfFOVX;
    float g_tanHalfFOVY;
    
    float3 g_cameraWorldPosition;
    float  g_cameraPassPadding;
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
};