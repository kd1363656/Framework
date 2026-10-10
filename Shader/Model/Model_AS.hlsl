#include "ModelMeshletCulling.hlsli"

groupshared ModelAmplificationPayload g_modelAmplificationPayload;
groupshared uint                      g_modelVisibleMeshletCount;

// Static / Skeletal 共通のAmplificationShader
// どちらも「メッシュのテーブルが指すバッファ」を読むだけなので、同じシェーダーで済む
// (Skeletalは、メッシュのテーブルがスキニング後の頂点と、計算し直したMeshletの境界を指している)
[numthreads(k_modelAmplificationShaderThreadCountX, k_modelAmplificationShaderThreadCountY, k_modelAmplificationShaderThreadCountZ)]
void main(const uint3 a_dispatchThreadID : SV_DispatchThreadID,
          const uint  a_groupThreadIndex : SV_GroupIndex)
{
    DispatchMesh(0U,
                 k_modelAmplificationDispatchMeshGroupCountY,
                 k_modelAmplificationDispatchMeshGroupCountZ,
                 g_modelAmplificationPayload);
}