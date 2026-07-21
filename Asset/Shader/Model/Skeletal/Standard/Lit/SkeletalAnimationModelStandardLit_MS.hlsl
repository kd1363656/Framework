#include "../../../Standard/ModelStandard.hlsli"
#include "../../SkeletalAnimationModel.hlsli"

[outputtopology("triangle")]
[numthreads(k_modelMeshShaderThreadCountX, k_modelMeshShaderThreadCountY, k_modelMeshShaderThreadCountZ)]
void main(out vertices MSOutput a_vertexList   [k_modelMaxMeshletVertexCount],
          out indices  uint3    a_primitiveList[k_modelMaxMeshletPrimitiveCount],
              const    uint3    a_groupID          : SV_GroupID, 
              const    uint     a_groupThreadIndex : SV_GroupIndex)
{
    // Compute Shaderが書き込んだ、
    // 現在Poseのスキニング済み頂点Buffer
    StructuredBuffer<SkeletalAnimationSkinnedVertex> l_skinnedVertexBuffer = ResourceDescriptorHeap[g_vertexBufferSRVDescriptorIndex];
    
    
}