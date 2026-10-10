#ifndef MODEL_STANDARD_UNLIT_MATERIAL_HLSLI
#define MODEL_STANDARD_UNLIT_MATERIAL_HLSLI
#include "../ModelStandardMaterial.hlsli"

struct ModelStandardUnLitMaterialData
{
    float4 baseColor;
    uint   baseColorTextureSRVDescriptorIndex;
};

ModelStandardUnLitMaterialData FetchModelStandardUnLitMaterialData()
{
    ModelStandardUnLitMaterialData l_material = (ModelStandardUnLitMaterialData)0;

    return l_material;
}

#endif // MODEL_STANDARD_UNLIT_MATERIAL_HLSLI