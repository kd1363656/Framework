#include "../../../../Model.hlsli"
#include "../../../../Shadow/Cascade/ModelCascadeShadow.hlsli"
#include "../../SkeletalAnimationModelGPUSkinning.hlsli"

// GPUスキニング用のメッシュシェーダー(カスケードシャドウ)
// 影は深度だけを書けばよいため、スキニング後の位置だけを使う
[outputtopology("triangle")]
[numthreads(k_modelMeshShaderThreadCountX, k_modelMeshShaderThreadCountY, k_modelMeshShaderThreadCountZ)]
void main(in  payload  ModelAmplificationPayload a_payload,
          out vertices MSOutputCascadeShadow     a_vertexList[k_modelMAXMeshletVertexCount],
          out indices  uint3                     a_primitiveList[k_modelMAXMeshletPrimitiveCount],
              const    uint3                     a_groupID : SV_GroupID,
              const    uint                      a_groupThreadIndex : SV_GroupIndex)
{
    StructuredBuffer<ModelMeshlet> l_modelMeshletBuffer      = ResourceDescriptorHeap[g_meshletBufferSRVDescriptorIndex];
    StructuredBuffer<uint>         l_uniqueVertexIndexBuffer = ResourceDescriptorHeap[g_uniqueVertexIndexBufferSRVDescriptorIndex];

    // AmplificationShaderから渡されたMeshletIndexを取得する
    const uint         l_meshletIndex = a_payload.meshletIndexList[a_groupID.x];
    const ModelMeshlet l_modelMeshlet = l_modelMeshletBuffer      [l_meshletIndex];

    SetMeshOutputCounts(l_modelMeshlet.vertexCount, l_modelMeshlet.triangleCount);

    for (uint l_vertexIndex = a_groupThreadIndex; l_vertexIndex < l_modelMeshlet.vertexCount; l_vertexIndex += k_modelMeshShaderThreadCountX)
    {
        const uint l_modelVertexIndex = l_uniqueVertexIndexBuffer[l_modelMeshlet.vertexOffset + l_vertexIndex];

        // 頂点を読み、今のアニメーションのコマに合わせてその場で変形する
        const GPUSkinnedVertex l_skinnedVertex = LoadGPUSkinnedVertex(l_modelVertexIndex);

        // World座標を、現在のCascadeに対応するLightClip空間へ変換する
        const float3 l_worldPosition = TransformModelLocalPositionToWorld(l_skinnedVertex.position);

        a_vertexList[l_vertexIndex].position = mul(float4(l_worldPosition, k_modelPositionElementW), g_cascadeViewProjectionMatrix);
    }

    for (uint l_triangleIndex = a_groupThreadIndex; l_triangleIndex < l_modelMeshlet.triangleCount; l_triangleIndex += k_modelMeshShaderThreadCountX)
    {
        a_primitiveList[l_triangleIndex] = FetchModelPackedPrimitiveIndex(l_modelMeshlet.triangleOffset + l_triangleIndex);
    }
}