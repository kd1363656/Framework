cbuffer CBSkeletalAnimationVertexSkinning : register(b0)
{
    uint g_sourceVertexBufferSRVDescriptorIndex;
    uint g_bonePaletteBufferSRVDescriptorIndex;
    uint g_boneMatrixBufferSRVDescriptorIndex;
    uint g_skinnedVertexBufferUAVDescriptorIndex;
    
    uint g_vertexCount;
}

struct SkeletalAnimationModelVertex
{
    float3 position;
    float3 normal;
    float4 tangent;
    float2 uv;
    
    float4 m_boneWeight;
    
    uint bonePaletteIndex0;
    uint bonePaletteIndex1;
    uint bonePaletteIndex2;
    uint bonePaletteIndex3;
};