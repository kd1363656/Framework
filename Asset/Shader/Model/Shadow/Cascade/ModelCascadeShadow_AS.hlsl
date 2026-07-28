#include "ModelCascadeShadowMeshletCulling.hlsli"

[numthreads(k_modelAmplificationShaderThreadCountX, k_modelAmplificationShaderThreadCountY, k_modelAmplificationShaderThreadCountZ)]
void main(const uint3 a_groupID : SV_GroupID)
{
    ModelAmplificationPayload l_payload = (ModelAmplificationPayload) 0;
    
    // 現在はASGroupがMeshlet1個を担当する
    const uint l_meshletIndex = a_groupID.x;

    const bool l_shouldDispatch = ShouldDispatchModelCascadeShadowMeshlet(l_meshletIndex);
    
    l_payload.meshletIndex = l_meshletIndex;

    const uint l_dispatchMeshGroupCountX = l_shouldDispatch ? k_modelAmplificationDispatchMeshGroupCountX : k_modelAmplificationDispatchMeshCulledGroupCountX;
    
    DispatchMesh(l_dispatchMeshGroupCountX,
                 k_modelAmplificationDispatchMeshGroupCountY,
                 k_modelAmplificationDispatchMeshGroupCountZ,
                 l_payload);
}