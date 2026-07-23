#include "../Model.hlsli"
#include "../ModelMeshletCulling.hlsli"

[numthreads(k_modelAmplificationShaderThreadCountX, k_modelAmplificationShaderThreadCountY, k_modelAmplificationShaderThreadCountZ)]
void main(uint3 a_groupID : SV_GroupID)
{
    ModelAmplificationPayload l_payload = (ModelAmplificationPayload) 0;

    const uint l_meshletIndex  = a_groupID.x;    
    const bool l_shoudDispatch = ShouldDispatchModelMeshlet(l_meshletIndex);
    
    l_payload.meshletIndex = l_meshletIndex;
    
    // Frustum外の場合はX方向のMeshShaderGroup数をゼロ西、
    // MeshShaderを起動しない
    const uint l_dispatchMeshGroupCountX = l_shoudDispatch ? k_modelAmplificationDispatchMeshGroupCountX : k_modelAmplificationDispatchMeshCulledGroupCountX;
    
    DispatchMesh(l_dispatchMeshGroupCountX, 
                 k_modelAmplificationDispatchMeshGroupCountY, 
                 k_modelAmplificationDispatchMeshGroupCountZ,
                 l_payload);
}