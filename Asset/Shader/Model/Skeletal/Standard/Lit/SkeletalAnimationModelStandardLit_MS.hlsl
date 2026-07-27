#include "../../../Standard/ModelStandard.hlsli"
#include "../../../Standard/Lit/ModelStandardLit.hlsli"
#include "../../SkeletalAnimationModel.hlsli"

[outputtopology("triangle")]
[numthreads(k_modelMeshShaderThreadCountX, k_modelMeshShaderThreadCountY, k_modelMeshShaderThreadCountZ)]
void main(in  payload  ModelAmplificationPayload a_payload,
          out vertices MSOutputLit               a_vertexList[k_modelMAXMeshletVertexCount],
          out indices  uint3                     a_primitiveList[k_modelMAXMeshletPrimitiveCount],
              const    uint                      a_groupThreadIndex : SV_GroupIndex)
{
    StructuredBuffer<SkeletalAnimationSkinnedVertex> l_skinnedVertexBuffer     = ResourceDescriptorHeap[g_vertexBufferSRVDescriptorIndex];
    StructuredBuffer<ModelMeshlet>                   l_modelMeshletBuffer      = ResourceDescriptorHeap[g_meshletBufferSRVDescriptorIndex];
    StructuredBuffer<uint>                           l_uniqueVertexIndexBuffer = ResourceDescriptorHeap[g_uniqueVertexIndexBufferSRVDescriptorIndex];

    // ASからPayload経由で渡されたMeshletIndexを使う
    const uint         l_meshletIndex = a_payload.meshletIndex;
    const ModelMeshlet l_modelMeshlet = l_modelMeshletBuffer[l_meshletIndex];
    
    // 出力頂点数、三角形数を設定
    SetMeshOutputCounts(l_modelMeshlet.vertexCount, l_modelMeshlet.triangleCount);
        
     for (uint l_vertexIndex = a_groupThreadIndex; l_vertexIndex < l_modelMeshlet.vertexCount; l_vertexIndex += k_modelMeshShaderThreadCountX)
    {
        // for分が回っている回数 + このメッシュレットの使用頂点開始位置からUniqueVertexIndexにアクセスするためのIndexを取得
        const uint l_uniqueVertexIndex = l_modelMeshlet.vertexOffset + l_vertexIndex;
        
        // UniqueVertexIndexBufferからVertexBufferにアクセスするためのIndexを取得
        // 例 : l_uniqueVertexIndexBuffer[0] = 40;でIndexが40のVertexBufferにアクセス
        const uint l_modelVertexIndex = l_uniqueVertexIndexBuffer[l_uniqueVertexIndex];
        
        // 取得した頂点番号から頂点情報を取得
        const SkeletalAnimationSkinnedVertex l_skeletalAnimationModelVertex = l_skinnedVertexBuffer[l_modelVertexIndex];
        
        // ワールド座標、法線、接線、ビュー座標を計算する
        const float3 l_worldPosition           = TransformModelLocalPositionToWorld(l_skeletalAnimationModelVertex.position);
        const float3 l_worldNormal             = TransformModelLocalNormalToWorld  (l_skeletalAnimationModelVertex.normal);
        const float4 l_worldTangent            = TransformModelLocalTangentToWorld (l_skeletalAnimationModelVertex.tangent);
        const float4 l_viewProjectionPosition  = mul                               (float4(l_worldPosition, k_modelPositionElementW), g_viewProjectionMatrix);

        a_vertexList[l_vertexIndex].position      = l_viewProjectionPosition;
        a_vertexList[l_vertexIndex].worldPosition = l_worldPosition;
        a_vertexList[l_vertexIndex].worldNormal   = l_worldNormal;
        a_vertexList[l_vertexIndex].worldTangent  = l_worldTangent;
        a_vertexList[l_vertexIndex].uv            = l_skeletalAnimationModelVertex.uv;
    }
    
    for (uint l_triangleIndex = a_groupThreadIndex; l_triangleIndex < l_modelMeshlet.triangleCount; l_triangleIndex += k_modelMeshShaderThreadCountX)
    {
        // 3個Pack方式では、uint32_t1個が三角形1個分のPrimitiveIndexを持つ
        // そのため、triangleOffsetはPack済みかPrimitiveIndexBuffer上の開始Indexとして扱う
        const uint l_packedPrimitiveIndex = l_modelMeshlet.triangleOffset + l_triangleIndex;
        
        // Packされたuint32_tから、元のuint8_tのPrimitiveIndexを3個取り出し、
        // MeshShaderの三角形Indexとしてuint3へ戻す   
        a_primitiveList[l_triangleIndex] = FetchModelPackedPrimitiveIndex(l_packedPrimitiveIndex);
    }
}