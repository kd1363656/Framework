#include "SpriteScreenShader.hlsli"

// 三角形2枚でポリゴン出力用四角形を出力
[outputtopology("triangle")]
[numthreads(k_spriteMeshShaderThreadCountX, k_spriteMeshShaderThreadCountY, k_spriteMeshShaderThreadCountZ)]
void main(out vertices MeshOutput a_vertexList   [k_spriteVertexCount],
		  out indices  uint3      a_primitiveList[k_spritePrimitiveCount])
{
	// SetMeshOutputCounts(出力頂点数、
	//					   出力プリミティブ数);
	SetMeshOutputCounts(k_spriteVertexCount, k_spritePrimitiveCount);

    uint l_textureWidth  = k_spriteTextureDefaultWidth;
    uint l_textureHeight = k_spriteTextureDefaultHeight;
	
    Texture2D<float4> l_baseColorTexture = ResourceDescriptorHeap[g_baseColorTextureSRVIndex];
	
	// GetDimensions(テクスチャの横幅、
	//				 テクスチャの縦幅);
    l_baseColorTexture.GetDimensions(l_textureWidth, l_textureHeight);

    // テクスチャの解像度を格納	
    const float2 l_textureSize = float2((float)l_textureWidth, (float)l_textureHeight);

	// sourceRECT指定あり : sourceRECTのサイズ
	// sourceRECT指定なし : テクスチャ全体サイ
    const float2 l_spriteSize = l_textureSize * g_scale;
	
	// pivotを基準にローカル座標を作成する
	// pivot = float2(0.0F, 0.0F) : 左上基準
	// pivot = float2(0.5F, 0.5F) : 中心基準
	// pivot = float2(1.0F, 1.0F) : 右下基準
    const float l_left   = -l_spriteSize.x * g_pivot.x;
    const float l_right  =  l_left         + l_spriteSize.x;
    const float l_top    =  l_spriteSize.y * g_pivot.y;
    const float l_bottom =  l_top          - l_spriteSize.y;
	
    // 左下、左上、右下、右上の座標を持つ配列を作成
    const float2 l_localPositionList[k_spriteVertexCount] =
    {
        float2(l_left,  l_bottom),
		float2(l_left,  l_top),
		float2(l_right, l_bottom),
		float2(l_right, l_top)
    };

	// UV座標を計算(g_sourceRECTからテクスチャのサイズを割るとUV座標が分かる)
    const float2 l_uvMIN = float2((float)g_sourceRECT.x,				  (float)g_sourceRECT.y)				  / l_textureSize;
    const float2 l_uvMAX = float2((float)g_sourceRECT.x + g_sourceRECT.z, (float)g_sourceRECT.y + g_sourceRECT.w) / l_textureSize;
	
	[unroll]
    for (uint l_i = 0U; l_i < k_spriteVertexCount; ++l_i)
    {
        const float2 l_worldPosition = l_localPositionList[l_i] + g_position;
		
		// 画面座標を正射影行列でクリップ座標へ変換する
        a_vertexList[l_i].position = mul(float4(l_worldPosition, k_spritePositionZ, k_spritePositionW), g_projectionMatrix);
    }
	
    a_vertexList[k_spritePrimitiveVertexIndexZero].uv  = float2(l_uvMIN.x, l_uvMAX.y);
    a_vertexList[k_spritePrimitiveVertexIndexOne].uv   = float2(l_uvMIN.x, l_uvMIN.y);
    a_vertexList[k_spritePrimitiveVertexIndexTwo].uv   = float2(l_uvMAX.x, l_uvMAX.y);
    a_vertexList[k_spritePrimitiveVertexIndexThree].uv = float2(l_uvMAX.x, l_uvMIN.y);

    a_primitiveList[k_spriteFirstPrimitiveIndex]  = uint3(k_spritePrimitiveVertexIndexZero, k_spritePrimitiveVertexIndexOne, k_spritePrimitiveVertexIndexTwo);
    a_primitiveList[k_spriteSecondPrimitiveIndex] = uint3(k_spritePrimitiveVertexIndexTwo,  k_spritePrimitiveVertexIndexOne, k_spritePrimitiveVertexIndexThree);
}