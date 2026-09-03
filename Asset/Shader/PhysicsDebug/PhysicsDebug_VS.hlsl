#include "PhysicsDebug.hlsli"

VSOutput main(VSInput a_input)
{
    VSOutput l_output = (VSOutput)0;
    
    l_output.position = mul(float4(a_input.position, k_physicsDebugPositionW), g_viewProjectionMatrix);
    l_output.color    = a_input.color;

    return l_output;
}