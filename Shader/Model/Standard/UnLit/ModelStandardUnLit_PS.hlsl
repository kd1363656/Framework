#include "../ModelStandard.hlsli"
#include "ModelStandardUnLit.hlsli"
#include "../../../Material/Standard/UnLit/StandardUnLitMaterial.hlsli"

float4 main(const MSOutputUnLit a_input) : SV_Target0
{
    const StandardUnLitMaterial l_material = FetchStandardUnLitMaterial(g_materialBufferSRVDescriptorIndex);

    return FetchModelBaseColor(l_material.baseColorTextureSRVDescriptorIndex, l_material.baseColorFactor, a_input.uv);
}