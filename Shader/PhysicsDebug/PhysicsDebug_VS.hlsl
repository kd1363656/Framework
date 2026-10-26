#include "PhysicsDebug.hlsli"

// 構造体を全て0で初期化するときに使う値
static const uint k_zeroInitializeValue = 0U;

VSOutput main(VSInput a_input)
{
    VSOutput l_output = (VSOutput)k_zeroInitializeValue;
    
    l_output.position = mul(float4(a_input.position, k_physicsDebugPositionW), g_viewProjectionMatrix);
    l_output.color    = a_input.color;

    return l_output;
}