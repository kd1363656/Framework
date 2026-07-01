#include "FinalColorShader.hlsli"

float4 main(VSOutput a_input) : SV_Target0
{
    return float4(1.0f, 0.0f, 1.0f, 1.0f);
    
    Texture2D<float4> l_finalColorTexture = ResourceDescriptorHeap[g_sceneColorTextureSRVIndex];
 
    // ガンマ補正済みテクスチャを描画
    return l_finalColorTexture.Sample(g_sceneColorSampler, a_input.uv);
}