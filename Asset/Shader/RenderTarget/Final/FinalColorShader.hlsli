#include "../FullScreenTriangleShader.hlsli"

SamplerState g_sceneColorSampler : register(s0);

cbuffer CBFinalColorPass : register(b0)
{
    uint   g_sceneColorTextureSRVIndex;
    float3 g_padding;
}

static const float k_finalColorMINValue = 0.0F;
static const float k_finalColorMAXValue = 1.0F;

static const float k_finalColorGammaBase = 2.2F;
static const float k_finalColorGamma     = 1.0F / k_finalColorGammaBase;

float3 ConvertLinearColorToSimpleSRGB(const float3 a_linearColor)
{
    const float3 l_clampedLinearColor = saturate(a_linearColor);
    
    return pow(l_clampedLinearColor, k_finalColorGamma);
}

float ConvertFinalColorAlpha(const float a_alpha)
{
    return saturate(a_alpha);
}