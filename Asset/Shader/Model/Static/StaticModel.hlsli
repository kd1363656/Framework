#ifndef STATIC_MODEL_HLSLI
#define STATIC_MODEL_HLSLI
#include "../Standard/ModelStandard.hlsli"

struct StaticModelVertex
{
    float3 position;
    float3 normal;
    float4 tangent;
    float2 uv;
};

#endif // STATIC_MODEL_HLSLI