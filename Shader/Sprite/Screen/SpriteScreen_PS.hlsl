#include "SpriteScreen.hlsli"

float4 main(VSOutput a_input) : SV_Target0
{   
    Texture2D<float4> l_baseColorTexture = ResourceDescriptorHeap[g_baseColorTextureSRVIndex];
    
    float4 l_outputColor = l_baseColorTexture.Sample(g_baseColorSampler, a_input.uv);
    
    // RGBAは乗算色として扱う
    l_outputColor *= g_color;
    
    // 最終的なAlpha値(テクスチャのAlpha × 乗算色のAlpha)が閾値未満ならピクセルを破棄
    // g_color.aだけで判定すると、テクスチャの透明部分が破棄されず
    // 透明なはずの部分が深度へ書き込まれて四角く残ってしまう
    if (l_outputColor.a < k_needDiscardWriteAlpha) { discard; }
    
    return l_outputColor;
}