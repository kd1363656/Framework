#include "../../../../Standard/ModelStandard.hlsli"
#include "../../../../Standard/Lit/ModelStandardLit.hlsli"
#include "../../SkeletalAnimationModelGPUSkinning.hlsli"

// GPUスキニング用のメッシュシェーダー(ライティングあり)
// StaticModelのメッシュシェーダーと同じ流れで、頂点を読む部分だけ「その場でスキニングする」処理に置き換えている
[outputtopology("triangle")]
[numthreads(k_modelMeshShaderThreadCountX, k_modelMeshShaderThreadCountY, k_modelMeshShaderThreadCountZ)]
void main(in  payload  ModelAmplificationPayload a_payload,
          out vertices MSOutputLit               a_vertexList[k_modelMAXMeshletVertexCount],
          out indices  uint3                     a_primitiveList[k_modelMAXMeshletPrimitiveCount],
              const    uint3                     a_groupID : SV_GroupID,
              const    uint                      a_groupThreadIndex : SV_GroupIndex)
{
    StructuredBuffer<ModelMeshlet> l_modelMeshletBuffer      = ResourceDescriptorHeap[g_meshletBufferSRVDescriptorIndex];
    StructuredBuffer<uint>         l_uniqueVertexIndexBuffer = ResourceDescriptorHeap[g_uniqueVertexIndexBufferSRVDescriptorIndex];

    // ASからPayload経由で渡されたMeshletIndexを使う
    const uint         l_meshletIndex = a_payload.meshletIndexList[a_groupID.x];
    const ModelMeshlet l_modelMeshlet = l_modelMeshletBuffer      [l_meshletIndex];

    // 出力頂点数、三角形数を設定
    SetMeshOutputCounts(l_modelMeshlet.vertexCount, l_modelMeshlet.triangleCount);

    for (uint l_vertexIndex = a_groupThreadIndex; l_vertexIndex < l_modelMeshlet.vertexCount; l_vertexIndex += k_modelMeshShaderThreadCountX)
    {
        // Meshlet内の頂点番号から、元の頂点Bufferの頂点番号を取得する
        const uint l_modelVertexIndex = l_uniqueVertexIndexBuffer[l_modelMeshlet.vertexOffset + l_vertexIndex];

        // 頂点を読み、今のアニメーションのコマに合わせてその場で変形する
        const GPUSkinnedVertex l_skinnedVertex = LoadGPUSkinnedVertex(l_modelVertexIndex);

        // ワールド座標、法線、接線、ビュー座標を計算する
        const float3 l_worldPosition          = TransformModelLocalPositionToWorld(l_skinnedVertex.position);
        const float3 l_worldNormal            = TransformModelLocalNormalToWorld  (l_skinnedVertex.normal);
        const float4 l_worldTangent           = TransformModelLocalTangentToWorld (l_skinnedVertex.tangent);
        const float4 l_viewProjectionPosition = mul                               (float4(l_worldPosition, k_modelPositionElementW), g_viewProjectionMatrix);

        a_vertexList[l_vertexIndex].position      = l_viewProjectionPosition;
        a_vertexList[l_vertexIndex].worldPosition = l_worldPosition;
        a_vertexList[l_vertexIndex].worldNormal   = l_worldNormal;
        a_vertexList[l_vertexIndex].worldTangent  = l_worldTangent;
        a_vertexList[l_vertexIndex].uv            = l_skinnedVertex.uv;
    }

    for (uint l_triangleIndex = a_groupThreadIndex; l_triangleIndex < l_modelMeshlet.triangleCount; l_triangleIndex += k_modelMeshShaderThreadCountX)
    {
        // 3個Pack方式のPrimitiveIndexから、三角形1個分の頂点番号を取り出す
        a_primitiveList[l_triangleIndex] = FetchModelPackedPrimitiveIndex(l_modelMeshlet.triangleOffset + l_triangleIndex);
    }
}