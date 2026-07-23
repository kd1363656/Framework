#include "../ModelMeshletCulling.hlsli"

// Model共通のAmplificationShader
[numthreads(k_modelAmplificationShaderThreadCountX, k_modelAmplificationShaderThreadCountY, k_modelAmplificationShaderThreadCountZ)]
void main(uint3 a_groupID : SV_GroupID)
{
    ModelAmplificationPayload l_payload = (ModelAmplificationPayload) 0;
    
    // CPU側が(meshletCount, 1, 1)を呼びため、
    // ASのa_groupID.xはそのままMeshletIndexとして使える。
    const uint l_meshletIndex = a_groupID.x;
    
    const bool l_shouldDispatch = ShouldDispatchModelMeshlet(l_meshletIndex);
    
    // 見えているMeshletだけMeshShaderにMeshletIndexを渡す
    l_payload.meshletIndex = l_meshletIndex;
    
    const uint l_dispatchMeshGroupCountX = l_shouldDispatch ? k_modelAmplificationDispatchMeshGroupCountX : k_modelAmplificationDispatchMeshCulledGroupCountX;
    
    DispatchMesh(l_dispatchMeshGroupCountX, 
                 k_modelAmplificationDispatchMeshGroupCountY, 
                 k_modelAmplificationDispatchMeshGroupCountZ, 
                 l_payload);
}