#ifndef MODEL_CASCADE_SHADOW_CULLING_HLSLI
#define MODEL_CASCADE_SHADOW_CULLING_HLSLI
#include "../../Model.hlsli"
#include "ModelCascadeShadow.hlsli"

// MeshletのBoundingSphereが、
// 現在のCascadeのLightOrthographic領域へ
// 少しでも重なっているか判定する。
bool IsVisiableModelMeshletByCAscadeFrustum(const ModelMeshletBounds a_meshletBounds)
{
    const float3 l_worldCenter     = TransformModelLocalPositionToWorld(a_meshletBounds.center);
    const float  l_worldRadius     = a_meshletBounds.radius * g_worldMaxScale;
    const float4 l_lightViewCenter = mul(float4(l_worldCenter, k_modelPositionElementW), g_cascadeViewMatrix);
    
    // Sphere全体がCascadeの左側または右側なら除外する
    if (l_lightViewCenter.x + l_worldRadius < g_cascadeOrthographicMIN.x ||
        l_lightViewCenter.x - l_worldRadius > g_cascadeOrthographicMAX.x)
    {
        return false;
    }

    // Sphere全体がCascadeの下側または上側なら除外する
    if (l_lightViewCenter.y + l_worldRadius < g_cascadeOrthographicMIN.y ||
        l_lightViewCenter.y - l_worldRadius > g_cascadeOrthographicMAX.y)
    {
        return false;
    }

    // Sphere全体がCascadeのNearよリ手前、
    // またはFarより奥なら除外する
    if (l_lightViewCenter.z + l_worldRadius < g_cascadeOrthographicMIN.z ||
        l_lightViewCenter.z - l_worldRadius > g_cascadeOrthographicMAX.z)
    {
        return false;
    }
 
    return true;
}

// DirectionalLightから見たとき
// Meshlet内の全てのTriagnleが裏向きか判定する
// OrthographicProjectionでは視線方向が位置によらず一定なので、
// Camera位置やBoundingSphere中心までの距離は使用しない
bool IsBackfaceModelMeshletByDirectionalLightCone(const ModelMeshletBounds a_mmeshletBounds)
{
    // 無効なConeではカリングしない
    if (a_mmeshletBounds.coneCutoff >= k_modelDisabledMeshletConeCutoff) { return false; }

    const float4 l_localConeAxis = float4(a_mmeshletBounds.coneAxis, k_modelDirectionElementW);
    const float3 l_worldConeAxis = normalize(mul(l_localConeAxis, g_worldInverseTransposeMatrix).xyz);
    
    // DirectionalLightが進む方向は
    // LightCameraからSceneを見るView方向と一致する
    return dot(g_directionalLightDirection, l_worldConeAxis) >= a_mmeshletBounds.coneCutoff;
}

bool ShouldDispatchModelCascadeShadowMeshlet(const uint a_meshletIndex)
{
    StructuredBuffer<ModelMeshletBounds> l_meshletBoundsBuffer = ResourceDescriptorHeap[g_meshletBoundsBufferSRVDescriptorIndex];
    
    const ModelMeshletBounds l_meshletBounds = l_meshletBoundsBuffer[a_meshletIndex];
    
    if (!IsVisiableModelMeshletByCAscadeFrustum(l_meshletBounds) ||
        IsBackfaceModelMeshletByDirectionalLightCone(l_meshletBounds))
    
    {
        return false;
    }
    
    return true;
}

#endif // MODEL_CASCADE_SHADOW_CULLING_HLSLI