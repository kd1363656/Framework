#include "SpriteScreenShader.hlsli"

float4 main(MeshOutput a_input) : SV_Target0
{
    // Alpha値が0.1以下ならピクセルを破棄
    if (g_color.a < k_needDiscardWriteAlpha)
    {
        discard;
    }
    
    Texture2D<float4> l_baseColorTexture = ResourceDescriptorHeap[g_baseColorTextureSRVIndex];
    
    float4 l_textureColor = l_baseColorTexture.Sample(g_baseColorSampler, a_input.uv);
    
    float4 l_outputColor = l_textureColor;

    // RGBAは乗算色として扱う
    l_outputColor *= g_color;
    
    return l_outputColor;
}