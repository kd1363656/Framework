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

static const uint k_finalPresentVertexCount    = 3U;
static const uint k_finalPresentPrimitiveCount = 1U;

static const uint k_finalPresentMeshShaderThreadCountX = 1U;
static const uint k_finalPresentMeshShaderThreadCountY = 1U;
static const uint k_finalPresentMeshShaderThreadCountZ = 1U;

static const uint k_finalPresentVertexIndexZero = 0U;
static const uint k_finalPresentVertexIndexOne  = 1U;
static const uint k_finalPresentVertexIndexTwo  = 2U;

SamplerState g_finalColorSampler : register(s0);