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

struct VSInput
{
    float3 position : POSITION;
    float4 color    : COLOR;
};

struct VSOutput
{
    float4 position : SV_Position;
    float4 color    : COLOR;
};

static const float k_physicsDebugPositionW = 1.0F;