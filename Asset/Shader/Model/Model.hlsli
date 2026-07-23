#ifndef MODEL_HLSLI
#define MODEL_HLSLI
#include "ModelMeshlet.hlsli"

static const float k_modelPositionElementW  = 1.0F;
static const float k_modelDirectionElementW = 0.0F;

// Frustumの側面Planeに対するSphere半径補正で使う
// sqrt(1.0 + tanFOV * tanFOV)の1.0部分。
static const float k_modelFrustumPlaneNormalBaseLength = 1.0F;

static const uint k_modelMeshShaderThreadCountX = 32U;
static const uint k_modelMeshShaderThreadCountY = 1U;
static const uint k_modelMeshShaderThreadCountZ = 1U;

static const uint k_modelAmplificationShaderThreadCountX = 1U;
static const uint k_modelAmplificationShaderThreadCountY = 1U;
static const uint k_modelAmplificationShaderThreadCountZ = 1U;

static const uint k_modelAmplificationDispatchMeshGroupCountX = 1U;
static const uint k_modelAmplificationDispatchMeshGroupCountY = 1U;
static const uint k_modelAmplificationDispatchMeshGroupCountZ = 1U;

static const uint k_modelAmplificationDispatchMeshCulledGroupCountX = 0U;

// AmplificationShaderからMeshShaderへ渡すPayload
// AS1グループ = Meshlet1個なので
// MeshShaderへ渡す情報は描画するMeshletIndexだけでよい
struct ModelAmplificationPayload
{
    uint meshletIndex;
};

cbuffer CBCameraPass : register(b0)
{
    row_major matrix g_viewMatrix;
    row_major matrix g_projectionMatrix;
    row_major matrix g_viewProjectionMatrix;
    
    float g_nearClip;
    float g_farClip;
    float g_tanHalfFOVX;
    float g_tanHalfFOVY;
    
    float3 g_cameraWorldPosition;
    float  g_cameraPassPadding;
};

cbuffer CBModelPerObject : register(b1)
{
    row_major matrix g_worldMatrix;
    
    row_major matrix g_worldInverseTransposeMatrix;
    
    float4 g_baseColorFactor;
    
    float g_roughnessFactor;
    float g_metallicFactor;
    uint  g_baseColorTextureSRVDescriptorIndex;
    uint  g_normalTextureSRVDescriptorIndex;
    
    uint g_metallicTextureSRVDescriptorIndex;
    uint g_roughnessTextureSRVDescriptorIndex;
    uint g_vertexBufferSRVDescriptorIndex;
    uint g_meshletBufferSRVDescriptorIndex;
    
    uint  g_uniqueVertexIndexBufferSRVDescriptorIndex;
    uint  g_primitiveIndexBufferSRVDescriptorIndex;
    uint  g_meshletBoundsBufferSRVDescriptorIndex;
    float g_worldMaxScale;
};

// StaticModelのLocal座標をWorld座標へ変換する
// PBRではライト方向やカメラ方向をWorld空間で計算するため、worldPositionが必要
float3 TransformModelLocalPositionToWorld(const float3 a_localPosition)
{
    const float4 l_localPosition = float4(a_localPosition, k_modelPositionElementW);
    const float4 l_worldPosition = mul   (l_localPosition, g_worldMatrix);

    return l_worldPosition.xyz;
}

#endif // MODEL_HLSLI