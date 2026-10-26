#include "../ModelAmplification.hlsli"

// SkeletalAnimationModelのAmplificationShader
// MeshletBoundsBufferは、ComputeShaderがスキニング後の形に合わせて毎フレーム作り直した物を使う
// カリングの処理はModelAmplification.hlsliにまとめている
[numthreads(k_modelAmplificationShaderThreadCountX, k_modelAmplificationShaderThreadCountY, k_modelAmplificationShaderThreadCountZ)]
void main(const uint3 a_dispatchThreadID : SV_DispatchThreadID,
          const uint  a_groupThreadIndex : SV_GroupIndex)
{
    DispatchModelAmplification(a_dispatchThreadID,
                               a_groupThreadIndex,
                               k_modelNoMeshletRadiusExpansion,
                               k_modelMeshletBackfaceCullingEnabled);
}