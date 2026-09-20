#include "../../../Model.hlsli"
#include "../../StaticModel.hlsli"
#include "../../../Shadow/Cascade/ModelCascadeShadow.hlsli"

[outputtopology("triangle")]
[numthreads(k_modelMeshShaderThreadCountX, k_modelMeshShaderThreadCountY, k_modelMeshShaderThreadCountZ)]
void main(in    payload  ModelAmplificationPayload a_payload,
          out   vertices MSOutputCascadeShadow     a_vertexList[k_modelMAXMeshletVertexCount],
          out   indices  uint3                     a_primitiveList[k_modelMAXMeshletPrimitiveCount],
                const    uint3                     a_groupID : SV_GroupID,
                const    uint                      a_groupThreadIndex : SV_GroupIndex)
{
    StructuredBuffer<StaticModelVertex> l_staticModelVertexBuffer = ResourceDescriptorHeap[g_vertexBufferSRVDescriptorIndex];
    StructuredBuffer<ModelMeshlet>      l_modelMeshletBuffer      = ResourceDescriptorHeap[g_meshletBufferSRVDescriptorIndex];
    StructuredBuffer<uint>              l_uniqueVertexIndexBuffer = ResourceDescriptorHeap[g_uniqueVertexIndexBufferSRVDescriptorIndex];
    
    // AmplificationShaderのカリングを通過したMeshletIndexを
    // Payloadから取得する
    const uint l_meshletIndex = a_payload.meshletIndexList[a_groupID.x];
    
    const ModelMeshlet l_modelMeshlet = l_modelMeshletBuffer[l_meshletIndex];
    
    // 現在のMeshletが出力する頂点数と三角形数を指定する
    SetMeshOutputCounts(l_modelMeshlet.vertexCount, l_modelMeshlet.triangleCount);
    
    // MeshShaderGroup内のthreadで、
    // Meshletが使用する頂点の変換を分担する
    for (uint l_vertexIndex = a_groupThreadIndex; l_vertexIndex < l_modelMeshlet.vertexCount; l_vertexIndex += k_modelMeshShaderThreadCountX)
    {
        // Meshlet内のVetrtex番号から、
        // UniqueVertexIndexBUfferへアクセスする位置を求める
        const uint l_uniqueVertexIndex = l_modelMeshlet.vertexOffset + l_vertexIndex;
        
        // 元のStaticModelVertexBufferへアクセスする
        // VertexIndexを取得する
        const uint l_modelVertexIndex = l_uniqueVertexIndexBuffer[l_uniqueVertexIndex];
        
        const StaticModelVertex l_staticModelVertex = l_staticModelVertexBuffer[l_modelVertexIndex];
        
        // StaticModelのLocal座標をWorld空間へ変換する
        const float3 l_worldPosition = TransformModelLocalPositionToWorld(l_staticModelVertex.position);
        
        // World座標を現在のCascadeに対応する
        // LightClip空間へ変換する
        a_vertexList[l_vertexIndex].position = mul(float4(l_worldPosition, k_modelPositionElementW), g_cascadeViewProjectionMatrix);
    }
    
    // MeshShaderGroup内のThreadで、
    // Meshletが持つ三角形Indexの出力を分担する
    for (uint l_triangleIndex = a_groupThreadIndex; l_triangleIndex < l_modelMeshlet.triangleCount; l_triangleIndex += k_modelMeshShaderThreadCountX)
    {
        const uint l_packedPrimitiveIndex = l_modelMeshlet.triangleOffset + l_triangleIndex;
        
        a_primitiveList[l_triangleIndex] = FetchModelPackedPrimitiveIndex(l_packedPrimitiveIndex);
    }
}