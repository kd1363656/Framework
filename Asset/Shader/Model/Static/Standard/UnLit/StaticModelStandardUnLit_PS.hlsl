#include "../../StaticModel.hlsli"

float4 main(const ModelMeshOutput a_input) : SV_Target0
{
    Texture2D<float4> l_baseColorTexture = ResourceDescriptorHeap[g_baseColorTextureSRVDescriptorIndex];
    
    const float4 l_baseColor = l_baseColorTexture.Sample(g_textureSampler, a_input.uv);
    
    return l_baseColor * g_baseColorFactor;
}