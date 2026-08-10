#include "FullScreenTriangle.hlsli"

VSOutput main(uint a_vertexID : SV_VertexID)
{
    VSOutput l_output = (VSOutput)0;
    
    l_output.position = float4(k_trianglePositionList[a_vertexID], k_triangleClipPositionZ, k_triangleClipPositionW);
    l_output.uv       = k_triangleUVList[a_vertexID];
    
    return l_output;
}