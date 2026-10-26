#ifndef MODEL_STANDARD_UNLIT_HLSLI
#define MODEL_STANDARD_UNLIT_HLSLI

struct MSOutputUnLit
{
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;

    // カリングの可視化用の判定結果(k_modelCullingDebugState～。通常は0 = 見えている)
    nointerpolation uint cullingDebugState : CULLING_DEBUG_STATE;
};

#endif // MODEL_STANDARD_UNLIT_HLSLI