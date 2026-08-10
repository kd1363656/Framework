#include "FinalColor.hlsli"

float4 main(VSOutput a_input) : SV_Target0
{
    Texture2D<float4> l_finalColorTexture = ResourceDescriptorHeap[g_sceneColorTextureSRVIndex];
 
    // ガンマ補正済みテクスチャを描画
    return l_finalColorTexture.Sample(g_sceneColorSampler, a_input.uv);
}