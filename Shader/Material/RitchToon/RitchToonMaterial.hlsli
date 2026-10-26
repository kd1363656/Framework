#ifndef RITCH_TOON_MATERIAL_HLSLI
#define RITCH_TOON_MATERIAL_HLSLI
#include "../Material.hlsli"

// RitchToonMaterialがCPUから送るデータ(トゥーン描画に必要なものだけを持つ)
// float3の後ろにfloatを1つ並べて、16byte(float4 1つ分)ごとに区切っている
// ※ C++側(Definition/Struct/Graphics/Buffer/Structured/RitchToonMaterialStruct.h)と必ず同じ並びにする
struct RitchToonMaterial
{
    float4 baseColorFactor;

    float3 shadowColor;
    float  shadowBorder;

    float3 matcapColor;
    float  shadowBlur;

    float3 specularColor;
    float  normalStrength;

    float3 emissionColor;
    float  smoothness;

    float3 outlineColor;
    float  outlineWidth;

    uint baseColorTextureSRVDescriptorIndex;
    uint normalTextureSRVDescriptorIndex;
    uint shadowMaskTextureSRVDescriptorIndex;
    uint matcapTextureSRVDescriptorIndex;

    uint matcapMaskTextureSRVDescriptorIndex;
    uint metallicMaskTextureSRVDescriptorIndex;
    uint emissionMaskTextureSRVDescriptorIndex;
    uint outlineMaskTextureSRVDescriptorIndex;

    // emissionBaseColorBlend : 発光色にメインテクスチャの色をどれだけ混ぜるか(0.0Fで発光色のまま、1.0Fで発光色 × メインテクスチャの色)
    // emissionReplaceRate    : 発光で元の色を置き換える割合(0.0Fで元の色に足す、1.0Fで発光マスクの所を発光色に置き換える)
    // padding～              : C++側と大きさを16byte単位で揃えるための詰め物
    float emissionBaseColorBlend;
    float emissionReplaceRate;
    float padding;
    float padding2;
};

// MaterialのGPUDataBufferから、RitchToonMaterialを取得する
RitchToonMaterial FetchRitchToonMaterial(const uint a_materialBufferSRVDescriptorIndex)
{
    StructuredBuffer<RitchToonMaterial> l_materialBuffer = ResourceDescriptorHeap[a_materialBufferSRVDescriptorIndex];

    return l_materialBuffer[k_materialBufferElementIndex];
}

#endif // RITCH_TOON_MATERIAL_HLSLI