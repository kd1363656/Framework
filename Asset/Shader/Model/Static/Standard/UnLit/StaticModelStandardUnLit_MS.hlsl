#include "../StaticModelStandardCommon.hlsli"

[outputtopology("triangle")]
[numthreads(k_modelMeshShaderThreadCountX, k_modelMeshShaderThreadCountY, k_modelMeshShaderThreadCountZ)]
void main(uint3 a_groupID : SV_GroupID,
          out   vertices ModelMeshOutput a_vertexList   [k_modelMaxMeshletVertexCount],
          out   indices  uint3           a_primitiveList[k_modelMaxMeshletPrimitiveCount])
{
    StructuredBuffer<StaticModelVertex> l_staticModelVertexBuffer = ResourceDescriptorHeap[g_vertexBufferSRVDescriptorIndex];
    StructuredBuffer<ModelMeshlet>      l_modelMeshletBuffer      = ResourceDescriptorHeap[g_meshletBufferSRVDescriptorIndex];
    StructuredBuffer<uint>              l_uniqueVertexIndexBuffer = ResourceDescriptorHeap[g_uniqueVertexIndexBufferSRVDescriptorIndex];
    StructuredBuffer<uint>              l_primitiveIndexBuffer    = ResourceDescriptorHeap[g_primitiveIndexBufferSRVDescriptorIndex];
    
    // メッシュレットインデックスはメッシュシェーダー起動回数
    const uint         l_meshletIndex = a_groupID.x;
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
        // 三角形一つが三頂点を使用するためIndex * 3を行う
        // 例 : l_modelMeshlet.triangleOffset = 0, l_triangleIndex = 1の場合
        // l_primitiveIndexOfsfetは3になりa_primitiveListはl_primitiveIndexBufferの3,4,5の配列にアクセスしてその値を格納する
        const uint l_primitiveIndexOffset = l_modelMeshlet.triangleOffset + l_triangleIndex * k_modelTriangleVertexCount;
        
        a_primitiveList[l_triangleIndex] = uint3
        (
            l_primitiveIndexBuffer[l_primitiveIndexOffset],
            l_primitiveIndexBuffer[l_primitiveIndexOffset + k_modelSecondPrimitiveVertexOffset],
            l_primitiveIndexBuffer[l_primitiveIndexOffset + k_modelThirdPrimitiveVertexOffset]
        );
    }
}