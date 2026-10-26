#include "../ModelRitchToon.hlsli"

// RitchToonの輪郭線を塗るPixelShader
// 輪郭線の色 = 輪郭線の色 × メインテクスチャの色 × ライトの色
// ・メインテクスチャの色を掛ける : 髪は髪の色、肌は肌の色の暗い線になり、どこでも黒一色の線より自然に見える
// ・ライトの色を掛ける           : 暗い場所で輪郭線だけが光って見えないようにする
float4 main(const MSOutputRitchToonOutline a_input) : SV_Target0
{
    const RitchToonMaterial l_material = FetchRitchToonMaterial(g_materialBufferSRVDescriptorIndex);

    const float4 l_baseColor = FetchModelBaseColor(l_material.baseColorTextureSRVDescriptorIndex, l_material.baseColorFactor, a_input.uv);

    const float3 l_outlineColor = l_material.outlineColor * l_baseColor.rgb * CalculateRitchToonLightColor();

    // カリングの可視化中は、消されたはずのMesh / Meshletに判定結果ごとの色を混ぜる(通常はそのままの色)
    return ApplyModelCullingDebugTint(float4(l_outlineColor, l_baseColor.a), a_input.cullingDebugState);
}