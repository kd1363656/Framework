#include "FinalPresentShader.hlsli"

float4 main(MeshOutput a_input) : SV_Target0
{
    Texture2D<float4> l_sceneColorTexture = ResourceDescriptorHeap[g_sceneColorTextureSRVIndex];
    
    // SceneColorはR16G16B16A16_FLOATのLinear色として扱う
    const float4 l_linearColor = l_sceneColorTexture.Sample(g_sceneColorSampler, a_input.uv);
    
    const float3 l_srgbColor = pow(saturate(l_linearColor.rgb), k_srgbGamma);

    return float4(l_srgbColor, l_linearColor.a);
}