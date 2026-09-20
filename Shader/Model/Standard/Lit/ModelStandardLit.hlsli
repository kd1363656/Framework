#ifndef MODEL_STANDARD_LIT_HLSLI
#define MODEL_STANDARD_LIT_HLSLI

struct MSOutputLit
{
    float4 position      : SV_Position;
    float3 worldPosition : POSITION0;
    float3 worldNormal   : NORMAL0;
    float4 worldTangent  : TANGENT0;
    float2 uv            : TEXCOORD0;
};

#endif // MODEL_STANDARD_LIT_HLSLI