#include "../Model.hlsli"
#include "SkeletalAnimationModel.hlsli"

cbuffer CBSkeletalAnimationMeshletBoundsUpdatePerObject : register(b0)
{
    uint g_meshletBoundsUpdateSkinnedVertexBufferSRVDescriptorIndex;
    uint g_meshletBoundsUpdateMeshletBufferSRVDescriptorIndex;
    uint g_meshletBoundsUpdateUniqueVertexIndexBufferSRVDescriptorIndex;
    uint g_meshletBoundsUpdateOutputBufferUAVDescriptorIndex;
}

static const uint k_meshletBoundsUpdateThreadCountX = 64U;
static const uint k_meshletBoundsUpdateThreadCountY = 1U;
static const uint k_meshletBoundsUpdateThreadCountZ = 1U;

// Meshletの頂点数を超えた未使用Threadを表す値。
static const uint k_invalidMeshletVertexThread = 0U;

static const float3 k_zeroPosition      = float3(0.0F, 0.0F, 0.0F);
static const float  k_zeroRadiusSquared = 0.0F;