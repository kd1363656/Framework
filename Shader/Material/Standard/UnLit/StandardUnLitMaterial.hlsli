#ifndef STANDARD_UNLIT_MATERIAL_HLSLI
#define STANDARD_UNLIT_MATERIAL_HLSLI
#include "../../Material.hlsli"

// StandardUnLitMaterialがCPUから送るデータ(ライティングしないため、ベースカラーだけを持つ)
// ※ C++側(Definition/Struct/Graphics/Buffer/Structured/StandardMaterialStruct.h)と必ず同じ並びにする
struct StandardUnLitMaterial
{
    float4 baseColorFactor;

    uint baseColorTextureSRVDescriptorIndex;
    uint padding;
    uint padding2;
    uint padding3;
};

// MaterialのGPUDataBufferから、StandardUnLitMaterialを取得する
StandardUnLitMaterial FetchStandardUnLitMaterial(const uint a_materialBufferSRVDescriptorIndex)
{
    StructuredBuffer<StandardUnLitMaterial> l_materialBuffer = ResourceDescriptorHeap[a_materialBufferSRVDescriptorIndex];

    return l_materialBuffer[k_materialBufferElementIndex];
}

#endif // STANDARD_UNLIT_MATERIAL_HLSLI