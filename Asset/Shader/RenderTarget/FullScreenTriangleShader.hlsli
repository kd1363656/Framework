struct VSOutput
{
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
};

static const float k_triangleClipPositionZ = 0.0F;
static const float k_triangleClipPositionW = 1.0F;

static const float k_trianglePositionXLeft   = -1.0F;
static const float k_trianglePositionXRight  =  3.0F;
static const float k_trianglePositionYTop    =  1.0F;
static const float k_trianglePositionYBottom = -3.0F;

static const float k_triangleUVMIN = 0.0F;
static const float k_triangleUVMAX = 2.0F;

static const uint k_triangleVertexCount = 3U;

static const uint k_triangleVertexIndexLeftTop    = 0U;
static const uint k_triangleVertexIndexRightTop   = 1U;
static const uint k_triangleVertexIndexLeftBottom = 2U;

// 左上、右上、左下のNDC座標での位置
static const float2 k_trianglePositionList[k_triangleVertexCount] = 
{
    float2(k_trianglePositionXLeft,  k_trianglePositionYTop),
    float2(k_trianglePositionXRight, k_trianglePositionYTop),
    float2(k_trianglePositionXLeft,  k_trianglePositionYBottom),
};

static const float2 k_triangleUVList[k_triangleVertexCount] = 
{
    float2(k_triangleUVMIN, k_triangleUVMIN),
    float2(k_triangleUVMAX, k_triangleUVMIN),
    float2(k_triangleUVMIN, k_triangleUVMAX),
};
