#ifndef MODEL_STANDARD_LIT_MATERIAL_HLSLI
#define MODEL_STANDARD_LIT_MATERIAL_HLSLI
#include "../ModelStandardMaterial.hlsli"

struct ModelStandardLitMaterialData
{
    float4 baseColor;
    float  metallic;
    float  roughness;
    uint   baseColorTextureSRVDescriptorIndex;
    uint   normalTextureSRVDescriptorIndex;
    uint   metallicTextureSRVDescriptorIndex;
    uint   roughnessTextureSRVDescriptorIndex;
};

ModelStandardLitMaterialData FetchModelStandardLitMaterialData()
{
    ModelStandardLitMaterialData l_material = (ModelStandardLitMaterialData)0;

    return l_material;
}

#endif // MODEL_STANDARD_LIT_MATERIAL_HLSLI