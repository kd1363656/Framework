#include "FinalPresentShader.hlsli"

// 全画面三角形を1枚だけ出力する
// 四角形二枚ではなく大きな三角形1枚で描画することで、
// 対角線の境界による補間差でのポストエフェクトのつなぎ目を避ける
[outputtopology("triangle")]
[numthreads(k_finalPresentMeshShaderThreadCountX, k_finalPresentMeshShaderThreadCountY, k_finalPresentMeshShaderThreadCountZ)]
void main(out vertices MeshOutput a_vertexList   [k_finalPresentVertexCount],
          out indices  uint3      a_primitiveList[k_finalPresentPrimitiveCount])
{
    SetMeshOutputCounts(k_finalPresentVertexCount, k_finalPresentPrimitiveCount);
    
    [unroll]
    for (uint l_i = 0U; l_i < k_finalPresentVertexCount; ++l_i)
    {
        a_vertexList[l_i].position = float4(k_finalPresentFullScreenTrianglePositionList[l_i], k_finalPresentClipPositionZ, k_finalPresentClipPositionW);
        a_vertexList[l_i].uv       = k_finalPresentFullScreenTriangleUVList[l_i];
    }

    a_primitiveList[k_finalPresentPrimitiveIndex] = uint3(k_finalPresentVertexIndexZero, k_finalPresentVertexIndexOne, k_finalPresentVertexIndexTwo);
}