#include "../ModelStandard.hlsli"
#include "ModelStandardUnLit.hlsli"
#include "../../../Material/Standard/UnLit/StandardUnLitMaterial.hlsli"

float4 main(const MSOutputUnLit a_input) : SV_Target0
{
    const StandardUnLitMaterial l_material = FetchStandardUnLitMaterial(g_materialBufferSRVDescriptorIndex);

    const float4 l_baseColor = FetchModelBaseColor(l_material.baseColorTextureSRVDescriptorIndex, l_material.baseColorFactor, a_input.uv);

    // カリングの可視化中は、消されたはずのMesh / Meshletに判定結果ごとの色を混ぜる(通常はそのままの色)
    return ApplyModelCullingDebugTint(l_baseColor, a_input.cullingDebugState);
}