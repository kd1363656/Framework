struct MSOutput
{
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
};

static const float k_positionZ = 0.0F;
static const float k_positionW = 1.0F;

static const float k_needDiscardWriteAlpha = 0.1F;

static const uint k_threadCountX = 1U;
static const uint k_threadCountY = 1U;
static const uint k_threadCountZ = 1U;

static const uint  k_vertexCount    = 4U;
static const uint  k_primitiveCount = 2U;

static const uint  k_textureDefaultWidth  = 0U;
static const uint  k_textureDefaultHeight = 0U;

static const uint  k_firstPrimitiveIndex  = 0U;
static const uint  k_secondPrimitiveIndex = 1U;

static const uint  k_primitiveVertexIndexZero  = 0U;
static const uint  k_primitiveVertexIndexOne   = 1U;
static const uint  k_primitiveVertexIndexTwo   = 2U;
static const uint  k_primitiveVertexIndexThree = 3U;

cbuffer CBSpritePass : register(b0)
{
    row_major matrix g_projectionMatrix;
}

cbuffer CBSpritePerObject : register(b1)
{
    float4 g_color;
    
    float2 g_position;
    float2 g_scale;
    
    float2 g_pivot;
    float2 g_firstPadding;
    
    uint4 g_sourceRECT;
    
    uint   g_baseColorTextureSRVIndex;
    float3 g_secondPadding;
}

SamplerState g_baseColorSampler : register(s0);