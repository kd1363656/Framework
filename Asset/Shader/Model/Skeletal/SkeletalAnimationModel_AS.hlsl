#include "../Model.hlsli"

[numthreads(k_modelAmplificationShaderThreadCountX, k_modelAmplificationShaderThreadCountY, k_modelAmplificationShaderThreadCountZ)]
void main(uint3 a_groupID : SV_GroupID)
{
    ModelAmplificationPayload l_payload = (ModelAmplificationPayload) 0;

    const uint l_meshletIndex = a_groupID.x;
    
    l_payload.meshletIndex = l_meshletIndex;
    
    DispatchMesh(k_modelAmplificationDispatchMeshGroupCountX, 
                 k_modelAmplificationDispatchMeshGroupCountY, 
                 k_modelAmplificationDispatchMeshGroupCountZ,
                 l_payload);
}