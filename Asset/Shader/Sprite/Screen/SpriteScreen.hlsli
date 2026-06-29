SamplerState g_baseColorSampler : register(s0);

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

struct VSOutput
{
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
};

static const float k_positionZ = 0.0F;
static const float k_positionW = 1.0F;

static const float k_needDiscardWriteAlpha = 0.1F;

static const float k_rectMIN = 0.0F;
static const float k_rectMAX = 1.0F;

static const uint k_vertexCount = 4U;

static const uint k_invalidSourceRECTSize = 0U;

static const float2 k_spriteScreenPositionRateList[k_vertexCount] =
{
	float2(k_rectMIN, k_rectMIN),
	float2(k_rectMAX, k_rectMIN),
	float2(k_rectMIN, k_rectMAX),
	float2(k_rectMAX, k_rectMAX),
};

static const float2 k_spriteScreenUVRateList[k_vertexCount] =
{
	float2(k_rectMIN, k_rectMIN),
	float2(k_rectMAX, k_rectMIN),
	float2(k_rectMIN, k_rectMAX),
	float2(k_rectMAX, k_rectMAX),
};