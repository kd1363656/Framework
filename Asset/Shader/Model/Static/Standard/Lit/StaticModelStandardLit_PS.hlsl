#include "../../StaticModel.hlsli"

float4 main(const ModelMeshOutput a_input) : SV_Target0
{
    const float3 l_normal = normalize(a_input.worldNormal);
    
    // g_directionalLightDirectionは光が進む方向として扱う。
    // 面からライトへ向かう方向は逆向きなので-directionにする。
    const float3 l_lightDirection = normalize(-g_directionalLightDirection);
    
    const float l_nDotL = saturate(dot(l_normal, l_lightDirection));
    
    return float4(l_nDotL.xxx, 1.0F);
}