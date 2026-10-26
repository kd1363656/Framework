#ifndef MODEL_MESHLET_CULLING_HLSLI
#define MODEL_MESHLET_CULLING_HLSLI
#include "../Camera/CameraPass.hlsli"
#include "Model.hlsli"

// ConeAxisを正規化できる長さか判定するための値
static const float k_modelMeshletConeAxisLengthSquaredEpsilon = k_modelMeshletCullingEpsilon * k_modelMeshletCullingEpsilon;

// Frustum側面PlaneでBoundingSphere判定するときの半径補正を計算する
float CalculateModelFrustumPlaneRadius(const float a_worldRadius, const float a_tanHalfFOV)
{
    return a_worldRadius * sqrt(k_modelFrustumPlaneNormalBaseLength + a_tanHalfFOV * a_tanHalfFOV);
}

// Meshlet単位のカリングは、CBCameraPassの「カリングに使うカメラ」の値(g_culling～)で判定する
// 通常は描画カメラと同じ値だが、エディターのデバッグカメラで描画している間はメインカメラの値が入るため、
// デバッグカメラから見ても「メインカメラが消しているMeshlet」がそのまま消える(又は色付きで描かれる)

// View空間のBoundingSphereがカメラのFrustumに入っているか判定する。
// 完全に外ならfalse
// 少しでも重なっているならtrue
bool IsVisibleViewSpaceBoundingSphere(const float3 a_viewCenter, const float a_worldRadius)
{
    // DirectXの左手系座標ではカメラ前方が+Z
    // Sphere全体がNearより手前に、Farより奥にあるなら見えない
    if (a_viewCenter.z + a_worldRadius < g_cullingNearClip ||
        a_viewCenter.z - a_worldRadius > g_cullingFarClip)
    {
        return false;
    }

    // カメラに近すぎるMeshletでは、center.zがnearClipより小さくなることがある
    // そのままcenter.z * tanFovを使うと、見える範囲が小さすぎて誤カリングする。
    // そこで、横幅・縦幅の判定に使うZは最低でもnearClipにする
    // これは近距離では安全側に倒して消しすぎない為の処理
    const float l_frustumTestDepth = max(a_viewCenter.z, g_cullingNearClip);
    
    // View空間のZ位置における、画面右端までの距離
    // Zが大きいほど、見える横幅は広がる
    const float l_halfViewWidth = l_frustumTestDepth * g_cullingTanHalfFOVX;
    
    // Frustum側面は斜めのPlaneなのでSphere半径を少し補正する
    const float l_horizontalPlaneRadius = CalculateModelFrustumPlaneRadius(a_worldRadius, g_cullingTanHalfFOVX);
    
    // Sphere全体が左外側、右外側にあるなら見えない。
    if (a_viewCenter.x + l_horizontalPlaneRadius < -l_halfViewWidth ||
        a_viewCenter.x - l_horizontalPlaneRadius >  l_halfViewWidth)
    {
        return false;
    }
    
    // View空間のz位置における、画面上端までの距離。
    // Zが大きいほど、みえる縦幅は広がる
    const float l_halfViewHeight      = l_frustumTestDepth * g_cullingTanHalfFOVY;
    const float l_verticalPlaneRadius = CalculateModelFrustumPlaneRadius(a_worldRadius, g_cullingTanHalfFOVY);
    
    // Sphere全体が下外側、上外側にあるなら見えない
    if (a_viewCenter.y + l_verticalPlaneRadius < -l_halfViewHeight ||
        a_viewCenter.y - l_verticalPlaneRadius >  l_halfViewHeight)
    {
        return false;
    }
    
    return true;
}

// カメラがMeshletのBoundingSphere内に入っているか判定する。
bool IsCameraInsideModelMeshletBoundingSphere(const float3 a_worldCenter, const float a_worldRadius)
{
    const float3 l_cameraToCenter        = a_worldCenter - g_cullingCameraWorldPosition;
    const float  l_cameraDistanceSquared = dot(l_cameraToCenter, l_cameraToCenter);

    // 境界付近の誤差で判定が揺れないよう、
    // Sphereを少し広げる
    const float l_safeWorldRadius = a_worldRadius + k_modelMeshletCullingEpsilon;
    
    return l_cameraDistanceSquared <= l_safeWorldRadius * l_safeWorldRadius;
}

// 指定したMeshletがFrustum内にあるか判定する
// a_worldRadiusExpansion : BoundingSphereの半径に足す距離(ワールド座標)
//                          輪郭線のように頂点を外側へ押し出して描く物は、押し出した分だけ範囲を広げて判定する
bool IsVisibleModelMeshletByFrustum(const ModelMeshletBounds a_modelMeshletBounds, const float a_worldRadiusExpansion)
{
    const float3 l_worldCenter = TransformModelLocalPositionToWorld(a_modelMeshletBounds.center);
    const float  l_worldRadius = a_modelMeshletBounds.radius * g_worldMaxScale + a_worldRadiusExpansion;
    
    // カメラがBoundingSphere内にいる場合は
    // NearPlaneやFrustum側面で誤カリングされないよう
    // Frustum内として扱う
    if (IsCameraInsideModelMeshletBoundingSphere(l_worldCenter, l_worldRadius)) { return true; }

    const float4 l_worldCenterPosition = float4(l_worldCenter, k_modelPositionElementW);
    
    const float4 l_viewCenter = mul(l_worldCenterPosition, g_cullingViewMatrix);
    
    return IsVisibleViewSpaceBoundingSphere(l_viewCenter.xyz, l_worldRadius);
}

// 指定されたMeshlet内のすべてのTriangleが
// カメラから見て裏向きか判定する
// StaticModelとSkeletalAnimationModelの両方で、
// BoundingSphereを使った判定式を共有する
// WorldMatrixには正の非ZeroScaleを使用することを前提とし、
// 負のScaleによるTriangleの表裏反転は考慮しない
bool IsBackfaceModelMeshletByCone(const ModelMeshletBounds a_meshletBounds)
{
    // Cutoffが無効の場合は
    // BackfaceConeCullingを行わない
    if (a_meshletBounds.coneCutoff >= k_modelDisabledMeshletConeCutoff) { return false; }

    const float l_coneAxisLengthSquared = dot(a_meshletBounds.coneAxis, a_meshletBounds.coneAxis);
    
    // ConeAxisがZero付近なら方向を決められないため
    // 描画する安全側へ倒す
    if (l_coneAxisLengthSquared <= k_modelMeshletConeAxisLengthSquaredEpsilon) { return false; }

    // g_worldInverseTransposeMatrixには
    // transpose(inverse(WorldMatrix))が格納されている
    // 再びtransposeしてWorld逆行列へ戻し
    // カメラをModelLocal空間へ変換する
    const float4 l_worldCameraPosition = float4(g_cullingCameraWorldPosition, k_modelPositionElementW);
    const float4 l_localCameraPosition = float4(mul(l_worldCameraPosition, transpose(g_worldInverseTransposeMatrix)));
    
    // BoundingSphere式で使用するCenter - Camera
    const float3 l_localCameraToCenter   = a_meshletBounds.center - l_localCameraPosition.xyz;
    const float  l_cameraDistanceSquared = dot(l_localCameraToCenter, l_localCameraToCenter);

    // 境界付近で見えているMeshletを消しにくくするため、
    // BoundingSphereを少し広げる
    const float l_safeBoundingSphereRadius = a_meshletBounds.radius + k_modelMeshletCullingEpsilon;
    
    // カメラがSphere内または境界付近にいる場合は、
    // Meshlet全体が裏向きだと安全に断定できない
    if (l_cameraDistanceSquared <= l_safeBoundingSphereRadius * l_safeBoundingSphereRadius) { return false; }

    const float l_cameraDistance        = sqrt (l_cameraDistanceSquared);
    const float l_inverseConeAxisLength = rsqrt(l_coneAxisLengthSquared);
    
    const float3 l_coneAxisDirection      = a_meshletBounds.coneAxis * l_inverseConeAxisLength;
    const float  l_cameraDirectionAxisDot = dot(l_localCameraToCenter, l_coneAxisDirection);
        
    // Perspective用BoundingSphere判定式
    // dot(Center - Camera, Axis) >= Cutoff * Distance + Radius
    // 条件を満たす場合、
    // Meshlet内のすべてのTriangleが裏向き
    const float l_backfaceCullingThreshold = a_meshletBounds.coneCutoff * l_cameraDistance + l_safeBoundingSphereRadius;
    
    return l_cameraDirectionAxisDot >= l_backfaceCullingThreshold;
}

// FrustumCullingとBackfaceConeCullingを実行し、Meshletの判定結果(k_modelCullingDebugState～)を返す
// 「見えている」以外ならMeshShaderを起動する必要が無い
// (カリングの可視化中は、見えていないMeshletも判定結果を付けて起動する)
// a_worldRadiusExpansion     : Frustumの判定でBoundingSphereの半径に足す距離(ワールド座標)
// a_isBackfaceCullingEnabled : falseなら裏向きのMeshletも消さない(輪郭線は裏面を描くため)
uint FetchModelMeshletCullingDebugState(const uint a_meshletIndex, const float a_worldRadiusExpansion, const bool a_isBackfaceCullingEnabled)
{
    StructuredBuffer<ModelMeshletBounds> l_meshletBoundsBuffer = ResourceDescriptorHeap[g_meshletBoundsBufferSRVDescriptorIndex];

    const ModelMeshletBounds l_meshletBounds = l_meshletBoundsBuffer[a_meshletIndex];

    // Frustum外ならBackface判定を行わず描画しない
    if (!IsVisibleModelMeshletByFrustum(l_meshletBounds, a_worldRadiusExpansion)) { return k_modelCullingDebugStateMeshletFrustumCulled; }

    // Frustum内でも全Triangleが裏向きなら描画しない
    if (a_isBackfaceCullingEnabled &&
        IsBackfaceModelMeshletByCone(l_meshletBounds))
    {
        return k_modelCullingDebugStateMeshletBackfaceCulled;
    }

    return k_modelCullingDebugStateVisible;
}

// FrustumCullingとBackfaceConeCullingを実行し、
// MeshShaderを起動する必要があるかを判定する
// a_worldRadiusExpansion     : Frustumの判定でBoundingSphereの半径に足す距離(ワールド座標)
// a_isBackfaceCullingEnabled : falseなら裏向きのMeshletも消さない
// a_outCullingDebugState     : Meshletの判定結果(カリングの可視化でMeshShaderへ渡す)
bool ShouldDispatchModelMeshlet(const uint  a_meshletIndex,
                                const float a_worldRadiusExpansion,
                                const bool  a_isBackfaceCullingEnabled,
                                out   uint  a_outCullingDebugState)
{
    a_outCullingDebugState = FetchModelMeshletCullingDebugState(a_meshletIndex, a_worldRadiusExpansion, a_isBackfaceCullingEnabled);

    // 見えているMeshletは常に起動する
    if (a_outCullingDebugState == k_modelCullingDebugStateVisible) { return true; }

    // カリングの可視化中は、消すはずのMeshletも判定結果を付けて起動する
    return g_isCullingDebugVisualize != k_modelFalseValue;
}

#endif // MODEL_MESHLET_CULLING_HLSLI