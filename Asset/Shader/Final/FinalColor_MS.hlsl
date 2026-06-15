#include "FinalColorShader.hlsli"

// 全画面三角形を1枚だけ出力する
// 四角形二枚ではなく大きな三角形1毎で描画することで、
// 対角線の協会による補完差ポストエフェクトのつなぎ目を避ける
[outputtopology("triangle")]
[numthreads(k_finalColorMeshShaderThreadCountX, k_finalColorMeshShaderThreadCountY, k_finalColorMeshShaderThreadCountZ)]
void main(out vertices MeshOutput a_vertexList   [k_finalColorVertexCount],
          out indices  uint3      a_primitiveList[k_finalColorPrimitiveCount])
{
    SetMeshOutputCounts(k_finalColorVertexCount, k_finalColorPrimitiveCount);
    
    const float2 l_positionList[k_finalColorVertexCount] = 
    {
        float2(-1.0F, -1.0F),
        float2(-1.0F,  3.0F),
        float2( 3.0F, -1.0F),
    };
    
    const float2 l_uvList[k_finalColorVertexCount] =
    {
        float2(0.0F,  1.0F),
        float2(0.0F, -1.0F),
        float2(2.0F,  1.0F),
    };

    [unroll]
    for (uint l_i = 0U; l_i < k_finalColorVertexCount; ++l_i)
    {
        a_vertexList[l_i].position = float4(l_positionList[l_i], 0.0F, 1.0F);
        a_vertexList[l_i].uv       = l_uvList[l_i];
    };

    a_primitiveList[0] = uint3(k_finalColorVertexIndexZero, k_finalColorVertexIndexOne, k_finalColorVertexIndexTwo);
}