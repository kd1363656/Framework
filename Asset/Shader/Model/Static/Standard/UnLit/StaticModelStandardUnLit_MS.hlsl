#include "../../StaticModel.hlsli"

[outputtopology("triangle")]
[numthreads(k_modelMeshShaderThreadCountX, k_modelMeshShaderThreadCountY, k_modelMeshShaderThreadCountZ)]
void main(in  payload  ModelAmplificationPayload a_payload,
          out vertices ModelMeshOutput           a_vertexList   [k_modelMaxMeshletVertexCount],
          out indices  uint3                     a_primitiveList[k_modelMaxMeshletPrimitiveCount],
                       uint3                     a_groupID : SV_GroupID)
{
    StructuredBuffer<StaticModelVertex> l_staticModelVertexBuffer = ResourceDescriptorHeap[g_vertexBufferSRVDescriptorIndex];
    StructuredBuffer<ModelMeshlet>      l_modelMeshletBuffer      = ResourceDescriptorHeap[g_meshletBufferSRVDescriptorIndex];
    StructuredBuffer<uint>              l_uniqueVertexIndexBuffer = ResourceDescriptorHeap[g_uniqueVertexIndexBufferSRVDescriptorIndex];
    
    // ASからPayload経由で渡されたMeshletIndexを使う
    const uint         l_meshletIndex = a_payload.meshletIndex;
    const ModelMeshlet l_modelMeshlet = l_modelMeshletBuffer[l_meshletIndex];
    
    // 出力頂点数、三角形数を設定
    SetMeshOutputCounts(l_modelMeshlet.vertexCount, l_modelMeshlet.triangleCount);
    
    for (uint l_vertexIndex = 0U; l_vertexIndex < l_modelMeshlet.vertexCount; ++l_vertexIndex)
    {
        // for分が回っている回数 + このメッシュレットの使用頂点開始位置からUniqueVertexIndexにアクセスするためのIndexを取得
        const uint l_uniqueVertexIndex = l_modelMeshlet.vertexOffset + l_vertexIndex;
        
        // UniqueVertexIndexBufferからVertexBufferにアクセスするためのIndexを取得
        // 例 : l_uniqueVertexIndexBuffer[0] = 40;でIndexが40のVertexBufferにアクセス
        const uint l_modelVertexIndex = l_uniqueVertexIndexBuffer[l_uniqueVertexIndex];
        
        // 取得した頂点番号から頂点情報を取得
        const StaticModelVertex l_staticModelVertex = l_staticModelVertexBuffer[l_modelVertexIndex];
        
        const float4 l_viewProjectionPosition = TransformStaticModelLocalPositionToViewProjection(l_staticModelVertex.position);
        
        a_vertexList[l_vertexIndex].position = l_viewProjectionPosition;
        a_vertexList[l_vertexIndex].uv       = l_staticModelVertex.uv;
    }
    
    for (uint l_triangleIndex = 0U; l_triangleIndex < l_modelMeshlet.triangleCount; ++l_triangleIndex)
    {
        // m_triangleOffsetの値はPack後のuint32_t配列Indexではなく、
        // Pack前のuint8_tのPrimitiveIndex配列上のbyteOffsetとして扱う。
        // 1三角形は3つのPrimitiveIndexを使うため、
        // triangleIndex * 3で、この三角形の先頭byteOffsetを求める
        const uint l_primitiveByteIndex = l_modelMeshlet.triangleOffset + l_triangleIndex * k_modelTriangleVertexCount;
        
        // Packされたuint32_tから、元のuint8_tのPrimitvieIndexを3個取り出し、
        // MeshShaderの三角形INdexとしてuint3へ戻す
        a_primitiveList[l_triangleIndex] = uint3
        (
            FetchStaticModelPackedPrimitiveIndex(l_primitiveByteIndex),
            FetchStaticModelPackedPrimitiveIndex(l_primitiveByteIndex + k_modelSecondPrimitiveVertexOffset),
            FetchStaticModelPackedPrimitiveIndex(l_primitiveByteIndex + k_modelThirdPrimitiveVertexOffset)
        );
    }
}