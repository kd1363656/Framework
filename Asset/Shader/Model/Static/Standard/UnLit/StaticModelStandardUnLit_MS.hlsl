#include "../StaticModelStandardCommon.hlsli"

[outputtopology["triangle"]]
[numthreads(k_modelMeshShaderThreadCountX, k_modelMeshShaderThreadCountY, k_modelMeshShaderThreadCountZ]
void main(uint3 a_groupID : SV_GroupID,
          out   vertices ModelMeshOutput a_vertexList   [k_modelMaxMeshletVertexCount],
          out indices uint3 a_primitiveList[k_modelMaxMeshletPrimitiveCount])
{
    StructuredBuffer<StaticModelVertex> l_staticModelVertexBuffer = ResourceDescriptorHeap[g_vertexBufferSRVDescriptorIndex];

}