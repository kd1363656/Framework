struct ModelMeshlet
{
    uint vertexOffset;
    uint vertexCount;
    uint triangleOffset;
    uint triangleCount;
};

// Meshlet1個分のカリング用境界情報
struct ModelMeshletBounds
{
    float3 center;
    float  radius;
    
    float3 coneApex;
    float  coneCutoff;
    
    float3 coneAxis;
    float  padding;
};

// AmplificationShaderからMeshShaderへ渡す値情報
// 「描画するMeshlet番号」だけを渡す
struct ModelMeshPayload
{
    uint meshletIndex;
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

static const uint k_modelTriangleVertexCount = 3U;

static const uint k_modelMaxMeshletVertexCount    = 64U;
static const uint k_modelMaxMeshletPrimitiveCount = 126U;

static const uint k_modelAmplificationShaderThreadCountX = 1U;
static const uint k_modelAmplificationShaderThreadCountY = 1U;
static const uint k_modelAmplificationShaderThreadCountZ = 1U;

static const uint k_modelMeshShaderThreadCountX = 1U;
static const uint k_modelMeshShaderThreadCountY = 1U;
static const uint k_modelMeshShaderThreadCountZ = 1U;

static const uint k_modelSecondPrimitiveVertexOffset = 1U;
static const uint k_modelThirdPrimitiveVertexOffset  = 2U;

static const float k_modelPositionVectorElementW  = 1.0F;
static const float k_modelDirectionVectorElementW = 0.0F;

static const float k_modelProjectionMatrixEpsilon = 0.0001F;
static const float k_modelFallbackFarClip         = 1000000.0F;

// meshoptimizerのConeCutoffは基本的に -1.0F ～ 1.0F の範囲。
// 1.0Fより大きい値なら無効扱いにする。
static const float k_modelInvalidConeCutoff = 1.0F;