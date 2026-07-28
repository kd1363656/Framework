#ifndef CASCADE_SHADOW_MAP_PASS_HLSLI
#define CASCADE_SHADOW_MAP_PASS_HLSLI

static const float k_cascadeShadowMapFullyLitVisibility = 1.0F;

static const float k_cascadeShadowMapTextureCoordinateScale = 0.5F;

static const float k_cascadeShadowMapTextureCoordinateBias = 0.5F;

static const float k_cascadeShadowMapMINDepth = 0.0F;
static const float k_cascadeShadowMapMAXDepth = 1.0F;

static const float k_cascadeShadowMapMINProjectionW = 0.000001F;

static const float k_cascadeShadowMapTexelSizeNumerator = 1.0F;

static const float k_cascadeShadowMapSampleCountIncrement = 1.0F;

static const uint k_cascadeShadowMapMAXCascadeCount     = 4U;
static const uint k_cascadeShadowMapInvalidCascadeCount = 0U;

static const uint k_cascadeShadowMapMIPLevel = 0U;
static const uint k_cascadeShadowMapInvalidTextureDimension = 0U;

static const int k_cascadeShadowMapPCFKernelRadius = 1;

// C++側のConstant::k_cascadeShadowMapDefaultMAXCascadeCountと同じ値にする

cbuffer CBCascadeShadowMapPass : register(b3)
{
    row_major matrix g_cascadeShadowMapViewProjectionMatrixList[k_cascadeShadowMapMAXCascadeCount];
    
    float4 g_cascadeShadowMapSplitDepthList;
    
    float g_cascadeShadowMapSampleDepthBias;
    float g_cascadeShadowMapPassPadding;
    uint  g_cascadeShadowMapSRVDescriptorIndex;
    uint  g_cascadeShadowMapCascadeCount;
}

// ShadowMapのDepth比較専用Sampler
SamplerComparisonState g_cascadeShadowMapComparisonSampler : register(s1);

uint FetchCascadeShadowMapIndex(const float3 a_worldPosition, const uint a_cascadeCount)
{
    // World座標をCameraView空間へ変換する
    // 左手系座標では、Cameraの前方のView空間ZをCameraからの奥行きとして利用できる
    const float4 l_viewPosition = mul(float4(a_worldPosition, k_model))

}

#endif // CASCADE_SHADOW_MAP_PASS_HLSLI