#include "../../../Model.hlsli"
#include "../../StaticModel.hlsli"
#include "../../../Shadow/Cascade/ModelCascadeShadow.hlsli"

[outputtopology("triangle")]
[numthreads(k_modelMeshShaderThreadCountX, k_modelAmplificationDispatchMeshGroupCountY, k_modelAmplificationDispatchMeshGroupCountZ)]
void main(out   vertices MSOutputCascadeShadow a_vertexList[k_modelMAXMeshletPrimitiveCount],
          out   indices  uint3                 a_primitiveList[k_modelMAXMeshletPrimitiveCount],
                const uint3                    a_groupID : SV_GroupID,
                const uint                     a_groupThreadIndex : SV_GroupIndex)
{
    StructuredBuffer<StaticModelVertex> l_staticModelVertexBuffer = ResourceDescriptorHeap[g_vertexBufferSRVDescriptorIndex];
    StructuredBuffer<ModelMeshlet>      l_modleMeshletBuffer      = ResourceDescriptorHeap[g_meshletBufferSRVDescriptorIndex];
    StructuredBuffer<uint>              l_uniqueVertexIndexBuffer = ResourceDescriptorHeap[g_uniqueVertexIndexBufferSRVDescriptorIndex];
    
    
    
    
}