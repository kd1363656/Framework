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

static const uint k_modelTriangleVertexCount = 3U;

static const uint k_modelMaxMeshletVertexCount    = 64U;
static const uint k_modelMaxMeshletPrimitiveCount = 126U;

static const uint k_modelMeshShaderThreadCountX = 1U;
static const uint k_modelMeshShaderThreadCountY = 1U;
static const uint k_modelMeshShaderThreadCountZ = 1U;

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