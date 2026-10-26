#include "FullScreenTriangle.hlsli"

// 構造体を全て0で初期化するときに使う値
static const uint k_zeroInitializeValue = 0U;

VSOutput main(uint a_vertexID : SV_VertexID)
{
    VSOutput l_output = (VSOutput)k_zeroInitializeValue;
    
    l_output.position = float4(k_trianglePositionList[a_vertexID], k_triangleClipPositionZ, k_triangleClipPositionW);
    l_output.uv       = k_triangleUVList[a_vertexID];
    
    return l_output;
}