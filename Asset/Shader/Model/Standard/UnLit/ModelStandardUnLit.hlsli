#ifndef MODEL_STANDARD_UNLIT_HLSLI
#define MODEL_STANDARD_UNLIT_HLSLI

struct MSOutputUnLit
{
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
};

#endif // MODEL_STANDARD_UNLIT_HLSLI