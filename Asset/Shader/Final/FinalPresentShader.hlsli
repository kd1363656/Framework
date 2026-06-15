struct MeshOutput
{
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
};

cbuffer CBFinalPresentPass : register(b0)
{
    uint   g_finalColorTextureSRVIndex;
    float3 g_padding;
}

static const float k_finalPresentClipPositionZ = 0.0F;
static const float k_finalPresentClipPositionW = 1.0F;

static const uint k_finalPresentVertexCount    = 3U;
static const uint k_finalPresentPrimitiveCount = 1U;

static const uint k_finalPresentPrimitiveIndex = 0U;

static const float2 k_finalPresentFullScreenTrianglePositionList[k_finalPresentVertexCount] =
{
    float2(-1.0F, -1.0F),
    float2(-1.0F,  3.0F),
    float2( 3.0F, -1.0F),
};

static const float2 k_finalPresentFullScreenTriangleUVList[k_finalPresentVertexCount] =
{
    float2(0.0F,  1.0F),
    float2(0.0F, -1.0F),
    float2(2.0F,  1.0F),
};

static const uint k_finalPresentMeshShaderThreadCountX = 1U;
static const uint k_finalPresentMeshShaderThreadCountY = 1U;
static const uint k_finalPresentMeshShaderThreadCountZ = 1U;

static const uint k_finalPresentVertexIndexZero = 0U;
static const uint k_finalPresentVertexIndexOne  = 1U;
static const uint k_finalPresentVertexIndexTwo  = 2U;

SamplerState g_finalColorSampler : register(s0);