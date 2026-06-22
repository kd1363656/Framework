struct ModelMeshlet
{
    uint vertexOffset;
    uint vertexCount;
    uint triangleOffset;
    uint triangleCount;
};

// StaticModelのMeshShaderからPixelShaderへ渡すSceneColor描画用出力
struct ModelMeshOutput
{
    float4 position      : SV_Position;
    float3 worldPosition : POSITION0;
    float3 worldNormal   : NORMAL0;
    float4 worldTangent  : TANGENT0;
    float2 uv            : TEXCOORD0;
};

// ModelのMeshlet1個分のカリング用境界情報
struct ModelMeshletBounds
{
    // Meshletを囲むBoundingSphereの中心。
    // meshopt_computeMeshletBounds由来なので、基本的にLocal空間
    float3 center;
    
    // BoundingSphereの半径
    // Local空間の半径なので、World空間ではg_worldMaxScaleを掛ける
    float radius;
    
    float3 coneApex;
    float  coneCutoff;
    
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

static const uint k_modelTriangleVertexCount = 3U;

static const uint k_modelMaxMeshletVertexCount    = 64U;
static const uint k_modelMaxMeshletPrimitiveCount = 126U;

static const uint k_modelMeshShaderThreadCountX = 1U;
static const uint k_modelMeshShaderThreadCountY = 1U;
static const uint k_modelMeshShaderThreadCountZ = 1U;

static const uint k_modelAmplificationShaderThreadCountX = 1U;
static const uint k_modelAmplificationShaderThreadCountY = 1U;
static const uint k_modelAmplificationShaderThreadCountZ = 1U;

static const uint k_modelAmplificationDispatchMeshGroupCountX = 1U;
static const uint k_modelAmplificationDispatchMeshGroupCountY = 1U;
static const uint k_modelAmplificationDispatchMeshGroupCountZ = 1U;

static const uint k_modelSecondPrimitiveVertexOffset = 1U;
static const uint k_modelThirdPrimitiveVertexOffset  = 2U;

static const float k_modelPositionElementW = 1.0F;

cbuffer CBCameraPass : register(b0)
{
    row_major matrix g_viewMatrix;
    row_major matrix g_projectionMatrix;
    row_major matrix g_viewProjectionMatrix;
    
    float g_nearClip;
    float g_farClip;
    float g_tanHalfFOVX;
    float g_tanHalfFOVY;
};

// View空間のBoundingSphereがカメラのFrustumに入っているか判定する。
// 完全に外ならfalse
// 少しでも重なっているならtrue
bool IsVisibleViewSpaceBoundingSphere(const float3 a_viewCenter, const float a_worldRadius)
{
    // DirectXの左手系座標ではカメラ前方が+Z
    // Sphere全体がNearより手前に、Farより奥にあるなら見えない
    if (a_viewCenter.z + a_worldRadius < g_nearClip ||
        a_viewCenter.z - a_worldRadius > g_farClip)
    {
        return false;
    }

    // View空間のZ位置における、画面右端までの距離
    // Zが大きいほど、見える横幅は広がる
    const float l_halfViewWidth = a_viewCenter.z * g_tanHalfFOVX;
    
    // Sphere全体が左外側、右外側にあるなら見えない。
    if (a_viewCenter.x + a_worldRadius < -l_halfViewWidth ||
        a_viewCenter.x - a_worldRadius >  l_halfViewWidth)
    {
        return false;
    }
    
    // View空間のz位置における、画面上端までの距離。
    // Zが大きいほど、みえる縦幅は広がる
    const float l_halfViewHeight = a_viewCenter.z * g_tanHalfFOVY;
    
    // Sphere全体が下外側、上外側にあるなら見えない
    if (a_viewCenter.y + a_worldRadius < -l_halfViewHeight ||
        a_viewCenter.y - a_worldRadius > l_halfViewHeight)
    {
        return false;
    }
    
    return true;
}
