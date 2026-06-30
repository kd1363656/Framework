#include "SpriteScreen.hlsli"

VSOutput main(uint a_vertexID : SV_VertexID)
{
    VSOutput l_output = (VSOutput) 0;
    
    Texture2D<float4> l_baseColorTexture = ResourceDescriptorHeap[g_baseColorTextureSRVIndex];

	uint l_textureWidth  = 0U;
	uint l_textureHeight = 0U;

	l_baseColorTexture.GetDimensions(l_textureWidth, l_textureHeight);

	const uint l_sourceX = g_sourceRECT.x;
	const uint l_sourceY = g_sourceRECT.y;

	const uint l_sourceWidth  = g_sourceRECT.z == k_invalidSourceRECTSize ? l_textureWidth  : g_sourceRECT.z;
	const uint l_sourceHeight = g_sourceRECT.w == k_invalidSourceRECTSize ? l_textureHeight : g_sourceRECT.w;

	const float2 l_sourceSize = { (float)l_sourceWidth, (float)l_sourceHeight, };

	const float2 l_spriteSize = l_sourceSize * g_scale;

	const float l_left   = g_position.x - l_spriteSize.x * g_pivot.x;
	const float l_top    = g_position.y + l_spriteSize.y * g_pivot.y;
	const float l_right  = l_left       + l_spriteSize.x;
	const float l_bottom = l_top        - l_spriteSize.y;

	const float2 l_positionLeftTop     = float2(l_left,  l_top);
	const float2 l_positionRightBottom = float2(l_right, l_bottom);

	const float2 l_uvMin = { (float)l_sourceX  / (float)l_textureWidth,					 (float)l_sourceY  / (float)l_textureHeight, };
	const float2 l_uvMax = { (float)(l_sourceX + l_sourceWidth) / (float)l_textureWidth, (float)(l_sourceY + l_sourceHeight) / (float)l_textureHeight, };

	const float2 l_positionRate = k_positionRateList[a_vertexID];
	const float2 l_uvRate       = k_uvRateList      [a_vertexID];

	const float2 l_position = lerp(l_positionLeftTop, l_positionRightBottom, l_positionRate);
	const float2 l_uv       = lerp(l_uvMin,           l_uvMax,              l_uvRate);

	// 投影行列を掛けてNDC座標に戻す
	l_output.position = mul(float4(l_position, k_positionZ, k_positionW), g_projectionMatrix);
	l_output.uv       = l_uv;

	return l_output;
}