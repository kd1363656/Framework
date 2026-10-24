#ifndef STANDARD_LIT_MATERIAL_HLSLI
#define STANDARD_LIT_MATERIAL_HLSLI
#include "../../Material.hlsli"

// StandardLitMaterialがCPUから送るデータ(Lit描画に必要なものだけを持つ)
// ※ C++側(Definition/Struct/Graphics/Buffer/Structured/StandardMaterialStruct.h)と必ず同じ並びにする
struct StandardLitMaterial
{
    float4 baseColorFactor;

    float roughnessFactor;
    float metallicFactor;
    uint  baseColorTextureSRVDescriptorIndex;
    uint  normalTextureSRVDescriptorIndex;

    uint metallicTextureSRVDescriptorIndex;
    uint roughnessTextureSRVDescriptorIndex;
    uint padding;
    uint padding2;
};

// MaterialのGPUDataBufferから、StandardLitMaterialを取得する
StandardLitMaterial FetchStandardLitMaterial(const uint a_materialBufferSRVDescriptorIndex)
{
    StructuredBuffer<StandardLitMaterial> l_materialBuffer = ResourceDescriptorHeap[a_materialBufferSRVDescriptorIndex];

    return l_materialBuffer[k_materialBufferElementIndex];
}

#endif // STANDARD_LIT_MATERIAL_HLSLI