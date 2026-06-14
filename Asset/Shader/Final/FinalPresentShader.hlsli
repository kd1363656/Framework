struct MeshOutput
{
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
};

cbuffer CBFinalPresentPass : register(b0)
{
    uint   g_sceneColorTextureSRVIndex;
    float3 g_padding;
}

// ガンマ補正に使う値
// テクスチャ読み込み時のフラグがDirectX::WIC_FLAGS_FORCE_SRGBだった場合
// サンプラーがsRGB空間からリニア空間に変換するため、そのリニア空間からsRGBに戻すためのガンマ補正が必要
// リニア空間で扱う理由はよりライティング処理を正確に行うため
// https://knowledge.shade3d.jp/knowledgebase/%E3%82%AC%E3%83%B3%E3%83%9E2-2%E3%81%A8srgb%E3%81%AE%E9%81%95%E3%81%84
static const float k_srgbGamma = 1.0 / 2.2F;

static const uint k_finalPresentVertexCount    = 3U;
static const uint k_finalPresentPrimitiveCount = 1U;

static const uint k_finalPresentMeshShaderThreadCountX = 1U;
static const uint k_finalPresentMeshShaderThreadCountY = 1U;
static const uint k_finalPresentMeshShaderThreadCountZ = 1U;

static const uint k_finalPresentVertexIndexZero = 0U;
static const uint k_finalPresentVertexIndexOne  = 1U;
static const uint k_finalPresentVertexIndexTwo  = 2U;

SamplerState g_sceneColorSampler : register(s0);