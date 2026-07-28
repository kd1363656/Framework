#ifndef CASCADE_SHADOW_MAP_PASS_HLSLI
#define CASCADE_SHADOW_MAP_PASS_HLSLI
#include "../../Model.hlsli"
#include "../../../Camera/CameraPass.hlsli"

static const float k_cascadeShadowMapFullyLitVisibility = 1.0F;
static const float k_cascadeShadowMapInitialVisibility  = 0.0F;
static const float k_cascadeShadowMapInitialSampleCount = 0.0F;

static const float k_cascadeShadowMapTextureCoordinateScale = 0.5F;

static const float k_cascadeShadowMapTextureCoordinateBias = 0.5F;

static const float k_cascadeShadowMapMINDepth = 0.0F;
static const float k_cascadeShadowMapMAXDepth = 1.0F;

static const float k_cascadeShadowMapMINProjectionW = 0.000001F;

static const float k_cascadeShadowMapTexelSizeNumerator = 1.0F;

static const float k_cascadeShadowMapSampleCountIncrement = 1.0F;

static const uint k_cascadeShadowMapMAXCascadeCount     = 4U;
static const uint k_cascadeShadowMapInvalidCascadeCount = 0U;

static const uint k_cascadeShadowMapMIPLevel                = 0U;
static const uint k_cascadeShadowMapInvalidTextureDimension = 0U;

static const uint k_cascadeShadowMapLastCascadeIndexOffset = 1U;

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
    const float4 l_viewPosition = mul(float4(a_worldPosition, k_modelPositionElementW), g_viewMatrix);
    const float  l_viewDepth    = l_viewPosition.z;
    
    // どの分割にも入らなかった場合は、最後のCascadeを使用する
    uint l_cascadeIndex = a_cascadeCount - k_cascadeShadowMapLastCascadeIndexOffset;

    for (uint l_checkCascadeIndex = 0U; l_checkCascadeIndex < a_cascadeCount; ++l_checkCascadeIndex)
    {
        const float l_splitDepth = g_cascadeShadowMapSplitDepthList[l_checkCascadeIndex];
        
        if (l_viewDepth <= l_splitDepth)
        {
            l_cascadeIndex = l_checkCascadeIndex;
            
            break;
        }
    }

    return l_cascadeIndex;
}

float CalculateCascadeShadowMapVisibility(const float3 a_worldPosition)
{
    // Shader側が対応できる最大数を超えないようにする
    const uint l_cascadeCount = min(g_cascadeShadowMapCascadeCount, k_cascadeShadowMapMAXCascadeCount);
    
    // Cascadeが一つも存在しない場合は
    // Shadowを適用せずに完全に明るい状態を返す
    if (l_cascadeCount == k_cascadeShadowMapInvalidCascadeCount) { return k_cascadeShadowMapFullyLitVisibility; }

    const uint l_cascadeIndex = FetchCascadeShadowMapIndex(a_worldPosition, l_cascadeCount);
    
    // World座標を、選択したCascadeのLightClip空間へ変換する
    const float4 l_lightClipPosition = mul(float4(a_worldPosition, k_modelPositionElementW), g_cascadeShadowMapViewProjectionMatrixList[l_cascadeIndex]);
    
    if (abs(l_lightClipPosition.w) <= k_cascadeShadowMapMINProjectionW) { return k_cascadeShadowMapFullyLitVisibility; }

    // Clip空間からNDC空間へ変換する
    const float3 l_lightNDCPosition = l_lightClipPosition.xyz / l_lightClipPosition.w;
    
    // DirectXのNDC座標をTextureUVへ変換する
    const float2 l_shadowMapUV = float2(l_lightNDCPosition.x * k_cascadeShadowMapTextureCoordinateScale + k_cascadeShadowMapTextureCoordinateBias, -l_lightNDCPosition.y * k_cascadeShadowMapTextureCoordinateScale + k_cascadeShadowMapTextureCoordinateBias);
    
    // DirectXのNDCDepthは0.0F ~ 1.0F
    // 範囲外の場合、このCascadeのShadowMapでは現在Pixelを判定できないため完全に明るくする
    if (l_lightNDCPosition.z  < k_cascadeShadowMapMINDepth ||
        l_lightClipPosition.z > k_cascadeShadowMapMAXDepth)
    {
        return k_cascadeShadowMapFullyLitVisibility;
    }
    
    // BindlessDescriptorHeapから
    // CascadeShadowMapのTexture2DArrayを取得する
    Texture2DArray<float> l_cascadeShadowMap = ResourceDescriptorHeap[g_cascadeShadowMapSRVDescriptorIndex];
    
    uint l_shadowMapWidth;
    uint l_shadowMapHeight;
    uint l_shadowMapArraySize;
    uint l_shadowMapMIPLevelCount;
    
    l_cascadeShadowMap.GetDimensions(k_cascadeShadowMapMIPLevel,
                                     l_shadowMapWidth,
                                     l_shadowMapHeight,
                                     l_shadowMapArraySize,
                                     l_shadowMapMIPLevelCount);
    
    // Texxture作成時に有効性は確認されているが
    // Shader内で範囲外Sliceを参照しないための確認を行う
    if (l_shadowMapWidth  == k_cascadeShadowMapInvalidTextureDimension ||
        l_shadowMapHeight == k_cascadeShadowMapInvalidTextureDimension ||
        l_cascadeIndex >= l_shadowMapArraySize)
    {
        return k_cascadeShadowMapFullyLitVisibility;
    }
    
    const float2 l_shadowMapTexelSize = k_cascadeShadowMapTexelSizeNumerator / float2(l_shadowMapWidth, l_shadowMapHeight);
    
    // ShadowAcneを軽減するため
    // 現在PixelのDepthを少しLight側へ移動する
    const float l_compareDepth = saturate(l_lightNDCPosition.z - g_cascadeShadowMapSampleDepthBias);
    
    float l_shadowVisibility  = k_cascadeShadowMapInitialVisibility;
    float l_shadowSampleCount = k_cascadeShadowMapInitialSampleCount;
    
    // 現在Texelと周囲のTexelを比較し
    // Shadow境界を滑らかにする
    for (int l_sampleOffsetY = -k_cascadeShadowMapPCFKernelRadius; l_sampleOffsetY <= k_cascadeShadowMapPCFKernelRadius; ++l_sampleOffsetY)
    {
        for (int l_sampleOffsetX = -k_cascadeShadowMapPCFKernelRadius; l_sampleOffsetX <= k_cascadeShadowMapPCFKernelRadius; ++l_sampleOffsetX)
        {
            const float2 l_sampleOffset = float2(l_sampleOffsetX, l_sampleOffsetY) * l_shadowMapTexelSize;
            
            // Texture2DArrayのZ成分にCascadeIndexを指定する
            // SampleCmpLevelZeroは、ShadowMapに保存されたDepthと
            // l_compareDepthをComparisonSamplerで比較し、
            // Lightから見えている割合を返す
            l_shadowVisibility + l_cascadeShadowMap.SampleCmpLevelZero(g_cascadeShadowMapComparisonSampler, float3(l_shadowMapUV + l_sampleOffset, l_cascadeIndex), l_compareDepth);
        }
    }
    
    return l_shadowVisibility / l_shadowSampleCount;
}

#endif // CASCADE_SHADOW_MAP_PASS_HLSLI