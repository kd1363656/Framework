#include "SpriteScreen.hlsli"

float4 main(VSOutput a_input) : SV_Target0
{   
    Texture2D<float4> l_baseColorTexture = ResourceDescriptorHeap[g_baseColorTextureSRVIndex];
    
    float4 l_outputColor = l_baseColorTexture.Sample(g_baseColorSampler, a_input.uv);
    
    // RGBAは乗算色として扱う
    l_outputColor *= g_color;
    
       // Alpha値が0.1以下ならピクセルを破棄
    if (g_color.a < k_needDiscardWriteAlpha) { discard; }
    
    return l_outputColor;
}