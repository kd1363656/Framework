#ifndef MODEL_STANDARD_LIT_HLSLI
#define MODEL_STANDARD_LIT_HLSLI

struct MSOutputLit
{
    float4 position      : SV_Position;
    float3 worldPosition : POSITION0;
    float3 worldNormal   : NORMAL0;
    float4 worldTangent  : TANGENT0;
    float2 uv            : TEXCOORD0;

    // カリングの可視化用の判定結果(k_modelCullingDebugState～。通常は0 = 見えている)
    // 三角形の中で補間すると意味が無くなるため、nointerpolationで代表頂点の値をそのまま使う
    nointerpolation uint cullingDebugState : CULLING_DEBUG_STATE;
};

#endif // MODEL_STANDARD_LIT_HLSLI