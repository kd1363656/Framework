#include "FinalPresentShader.hlsli"

float4 main(MeshOutput a_input) : SV_Target0
{
    Texture2D<float4> l_finalColorTexture = ResourceDescriptorHeap[g_finalColorTextureSRVIndex];
    
    // FinalColorはFinalColorPassでLinear -> sRGB変換済み
    // ここでpowやsRGB変換を行うと二重補正になるため、そのままBackBufferへ出力する。
    return l_finalColorTexture.Sample(g_finalColorSampler, a_input.uv);
}