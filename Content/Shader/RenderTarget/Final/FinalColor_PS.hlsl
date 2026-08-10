#include "FinalColor.hlsli"

float4 main(VSOutput a_input) : SV_Target0
{
    Texture2D<float4> l_sceneColorTexture = ResourceDescriptorHeap[g_sceneColorTextureSRVIndex];
    
    const float4 l_linearColor = l_sceneColorTexture.Sample(g_sceneColorSampler, a_input.uv);

    // シーンカラーテクスチャの色空間をリニアからsRGBに変換して出力する
    const float3 l_outputColorRGB = ConvertLinearColorToSimpleSRGB(l_linearColor.rgb);
    const float  l_outputAlpha    = ConvertFinalColorAlpha        (l_linearColor.a);
    
    return float4(l_outputColorRGB, l_outputAlpha);
}