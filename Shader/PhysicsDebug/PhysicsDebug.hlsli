#ifndef PHYSICS_DEBUG_HLSLI
#define PHYSICS_DEBUG_HLSLI
#include "../Camera/CameraPass.hlsli"

static const float k_physicsDebugPositionW = 1.0F;

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

#endif // PHYSICS_DEBUG_HLSLI