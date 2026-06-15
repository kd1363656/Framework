#include "FinalColorShader.hlsli"

// 全画面三角形を1枚だけ出力する
// 四角形二枚ではなく大きな三角形1枚で描画することで、
// 対角線の境界による補間差でのポストエフェクトのつなぎ目を避ける
[outputtopology("triangle")]
[numthreads(k_finalColorMeshShaderThreadCountX, k_finalColorMeshShaderThreadCountY, k_finalColorMeshShaderThreadCountZ)]
void main(out vertices MeshOutput a_vertexList   [k_finalColorVertexCount],
          out indices  uint3      a_primitiveList[k_finalColorPrimitiveCount])
{
    SetMeshOutputCounts(k_finalColorVertexCount, k_finalColorPrimitiveCount);
    
    [unroll]
    for (uint l_i = 0U; l_i < k_finalColorVertexCount; ++l_i)
    {
        a_vertexList[l_i].position = float4(k_finalColorFullScreenTrianglePositionList[l_i], k_finalColorClipPositionZ, k_finalColorClipPositionW);
        a_vertexList[l_i].uv       = k_finalColorFullScreenTriangleUVList[l_i];
    }

    a_primitiveList[k_finalColorPrimitiveIndex] = uint3(k_finalColorVertexIndexZero, k_finalColorVertexIndexOne, k_finalColorVertexIndexTwo);
}