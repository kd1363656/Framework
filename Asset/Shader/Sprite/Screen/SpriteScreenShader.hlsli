struct MeshOutput
{
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
};

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

static const float k_spriteUVMIN = 0.0F;
static const float k_spriteUVMAX = 1.0F;

static const float k_spritePositionZ = 0.0F;
static const float k_spritePositionW = 1.0F;

static const float k_needDiscardWriteAlpha = 0.1F;

static const uint  k_spriteMeshShaderThreadCountX = 1U;
static const uint  k_spriteMeshShaderThreadCountY = 1U;
static const uint  k_spriteMeshShaderThreadCountZ = 1U;

static const uint  k_spriteVertexCount    = 4U;
static const uint  k_spritePrimitiveCount = 2U;

static const uint  k_spriteTextureDefaultWidth    = 0U;
static const uint  k_spriteTextureDefaultHeight   = 0U;
static const uint  k_spriteSourceRECTInvalidSize  = 0U;

static const uint  k_spriteLeftBottomVertexIndex  = 0U;
static const uint  k_spriteLeftTopVertexIndex     = 1U;
static const uint  k_spriteRightBottomVertexIndex = 2U;
static const uint  k_spriteRightTopVertexIndex    = 3U;

static const uint  k_spriteFirstPrimitiveIndex  = 0U;
static const uint  k_spriteSecondPrimitiveIndex = 1U;

static const uint  k_spritePrimitiveVertexIndexZero  = 0U;
static const uint  k_spritePrimitiveVertexIndexOne   = 1U;
static const uint  k_spritePrimitiveVertexIndexTwo   = 2U;
static const uint  k_spritePrimitiveVertexIndexThree = 3U;

SamplerState g_baseColorSampler : register(s0);