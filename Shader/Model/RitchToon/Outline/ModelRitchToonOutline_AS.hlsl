#include "../../ModelAmplification.hlsli"
#include "../../../Material/RitchToon/RitchToonMaterial.hlsli"

// ※ ModelRitchToon.hlsliはテクスチャのSamplerを宣言しているため、ここでは読み込まない
//   (RootSignatureはAmplificationShaderにSamplerを渡していないため、宣言があるとPipelineStateの作成に失敗する)

// RitchToonの輪郭線を描くAmplificationShader(StaticModel、SkeletalAnimationModel、GPUスキニングで共通)
// 通常の描画との違いは2つだけ
//   ・輪郭線は裏面を外側へ押し出して描くため、裏向きのMeshletを消さない
//   ・頂点を太さの分だけ外側へ押し出すため、Meshletの範囲(BoundingSphere)を太さの分だけ広げて判定する
//     (広げないと、画面の端で輪郭線だけが欠けることがある)
[numthreads(k_modelAmplificationShaderThreadCountX, k_modelAmplificationShaderThreadCountY, k_modelAmplificationShaderThreadCountZ)]
void main(const uint3 a_dispatchThreadID : SV_DispatchThreadID,
          const uint  a_groupThreadIndex : SV_GroupIndex)
{
    // 輪郭線の太さはMaterialが持っている(このShaderはRitchToonMaterialのMeshにだけ使われる)
    const RitchToonMaterial l_material = FetchRitchToonMaterial(g_materialBufferSRVDescriptorIndex);

    DispatchModelAmplification(a_dispatchThreadID,
                               a_groupThreadIndex,
                               l_material.outlineWidth,
                               k_modelMeshletBackfaceCullingDisabled);
}