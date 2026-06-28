#include "FullScreenTriangleShader.hlsli"

VSOuput main(uint a_vertexID : SV_VertexID)
{
    VSOuput l_output = (VSOuput)0;
    
    l_output.position = float4(k_trianglePositionList[a_vertexID], k_triangleClipPositionZ, k_triangleClipPositionW);
    l_output.uv       = k_triangleUVList[a_vertexID];
    
    return l_output;
}