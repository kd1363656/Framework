struct MeshOutput
{
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
};

cbuffer CBFinalColorPass : register(b0)
{
    uint   g_sceneColorTextureSRVIndex;
    float3 g_padding;
}

// ガンマ補正に使う値
// テクスチャ読み込み時のフラグがDirectX::WIC_FLAGS_FORCE_SRGBだった場合
// サンプラーがsRGB空間からリニア空間に変換するため、そのリニア空間からsRGBに戻すためのガンマ補正が必要
// リニア空間で扱う理由はよりライティング処理を正確に行うため
// ただし厳密なsRGB変換ではない
// https://knowledge.shade3d.jp/knowledgebase/%E3%82%AC%E3%83%B3%E3%83%9E2-2%E3%81%A8srgb%E3%81%AE%E9%81%95%E3%81%84
static const float k_srgbGamma = 1.0F / 2.2F;

static const float k_finalColorClipPositionZ = 0.0F;
static const float k_finalColorClipPositionW = 1.0F;

static const uint k_finalColorVertexCount    = 3U;
static const uint k_finalColorPrimitiveCount = 1U;

static const float2 k_finalColorFullScreenTrianglePositionList[k_finalColorVertexCount] =
{
    float2(-1.0F, -1.0F),
    float2(-1.0F,  3.0F),
    float2( 3.0F, -1.0F),
};

static const float2 k_finalColorFullScreenTriangleUVList[k_finalColorVertexCount] =
{
    float2(0.0F,  1.0F),
    float2(0.0F, -1.0F),
    float2(2.0F,  1.0F),
};

static const uint k_finalColorPrimitiveIndex = 0U;

static const uint k_finalColorMeshShaderThreadCountX = 1U;
static const uint k_finalColorMeshShaderThreadCountY = 1U;
static const uint k_finalColorMeshShaderThreadCountZ = 1U;

static const uint k_finalColorVertexIndexZero = 0U;
static const uint k_finalColorVertexIndexOne  = 1U;
static const uint k_finalColorVertexIndexTwo  = 2U;

SamplerState g_sceneColorSampler : register(s0);

