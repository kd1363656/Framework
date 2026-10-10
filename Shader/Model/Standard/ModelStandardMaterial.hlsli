#ifndef MODEL_STANDARD_MATERIAL_HLSLI
#define MODEL_STANDARD_MATERIAL_HLSLI

cbuffer RCModelMaterialTable : register(b6)
{
    uint g_materialTableSRVDescriptorIndex;
};

#endif // MODEL_STANDARD_MATERIAL_HLSLI