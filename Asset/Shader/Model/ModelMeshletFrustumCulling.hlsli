#ifndef MODEL_MESHLET_FRUSTUM_CULLING_HLSLI
#define MODEL_MESHLET_FRUSTUM_CULLING_HLSLI
#include "Model.hlsli"

// Frustum側面PlaneでBoundingSphere判定するときの半径補正を計算する
float CalculateModelFrustumPlaneRadius(const float a_worldRadius, const float a_tanHalfFOV)
{
    return a_worldRadius * sqrt(k_modelFrustumPlaneNormalBaseLength + a_tanHalfFOV * a_tanHalfFOV);
}

// View空間のBoundingSphereがカメラのFrustumに入っているか判定する。
// 完全に外ならfalse
// 少しでも重なっているならtrue
bool IsVisibleViewSpaceBoundingSphere(const float3 a_viewCenter, const float a_worldRadius)
{
    // DirectXの左手系座標ではカメラ前方が+Z
    // Sphere全体がNearより手前に、Farより奥にあるなら見えない
    if (a_viewCenter.z + a_worldRadius < g_nearClip ||
        a_viewCenter.z - a_worldRadius > g_farClip)
    {
        return false;
    }

    // カメラに近すぎるMeshletでは、center.zがnearClipより小さくなることがある
    // そのままcenter.z * tanFovを使うと、見える範囲が小さすぎて誤カリングする。
    // そこで、横幅・縦幅の判定に使うZは最低でもnearClipにする
    // これは近距離では安全側に倒して消しすぎない為の処理
    const float l_frustumTestDepth = max(a_viewCenter.z, g_nearClip);
    
    // View空間のZ位置における、画面右端までの距離
    // Zが大きいほど、見える横幅は広がる
    const float l_halfViewWidth = l_frustumTestDepth * g_tanHalfFOVX;
    
    // Frustum側面は斜めのPlaneなのでSphere半径を少し補正する
    const float l_horizontalPlaneRadius = CalculateModelFrustumPlaneRadius(a_worldRadius, g_tanHalfFOVX);
    
    // Sphere全体が左外側、右外側にあるなら見えない。
    if (a_viewCenter.x + l_horizontalPlaneRadius < -l_halfViewWidth ||
        a_viewCenter.x - l_horizontalPlaneRadius >  l_halfViewWidth)
    {
        return false;
    }
    
    // View空間のz位置における、画面上端までの距離。
    // Zが大きいほど、みえる縦幅は広がる
    const float l_halfViewHeight      = l_frustumTestDepth * g_tanHalfFOVY;
    const float l_verticalPlaneRadius = CalculateModelFrustumPlaneRadius(a_worldRadius, g_tanHalfFOVY);
    
    // Sphere全体が下外側、上外側にあるなら見えない
    if (a_viewCenter.y + l_verticalPlaneRadius < -l_halfViewHeight ||
        a_viewCenter.y - l_verticalPlaneRadius >  l_halfViewHeight)
    {
        return false;
    }
    
    return true;
}

// カメラがMeshletのBoundingSphere内に入っているか判定する。
bool IsCameraInsideModelMeshletBoundingSphere(const ModelMeshletBounds a_meshletBounds)
{
    // Meshletの中心座標はLocal空間なのでWorld空間へ変換する
    const float3 l_worldCenter = TransformModelLocalPositionToWorld(a_meshletBounds.center);
    
    // Meshletの半径はLocal空間なので、World最大スケールを掛けてWorld空間の半径にする
    const float l_worldRadius = a_meshletBounds.radius * g_worldMaxScale;
    
    // カメラ位置とMeshlet中心の距離を求める
    const float l_cameraDistance = length(l_worldCenter - g_cameraWorldPosition);
    
    // カメラがBoundingSphere内にいるならtrue
    // epsilonを足しているのは、境界付近の誤差でカリングが暴れないようにするため。
    return l_cameraDistance <= l_worldRadius + k_modelMeshletCullingEpsilon;
}

// 指定したMeshletがFrustum内にあるか判定する
bool IsVisibleModelMeshletByFrustum(const uint a_meshletIndex)
{
    // MeshletごとのBoundingSphere情報を読む
    StructuredBuffer<ModelMeshletBounds> l_meshletBoundsBuffer = ResourceDescriptorHeap[g_meshletBoundsBufferSRVDescriptorIndex];

    const ModelMeshletBounds l_meshletBounds = l_meshletBoundsBuffer[a_meshletIndex];
    
    // カメラがMeshletのBoundingSphere内にいる場合、
    // NearPlaneやFrustum側面で誤カリングされやすいのでFrustum内として扱う
    if (IsCameraInsideModelMeshletBoundingSphere(l_meshletBounds)) { return true; }
    
    // MeshletBoundsのcenterはLocal空間。
    // まずWorld空間へ変換する
    const float4 l_localCenter = float4(l_meshletBounds.center, k_modelPositionElementW);
    const float4 l_worldCenter = mul(l_localCenter, g_worldMatrix);
    
    // World空間からView空間へ変換する。
    const float4 l_viewCenter = mul(l_worldCenter, g_viewMatrix);
    
    // BoundingSphereの半径もWorld空間に合わせる
    // MeshletBoundsのradiusはLocal空間の半径
    // モデルが拡大されている場合、radiusも拡大する必要がある
    // 非均一スケールの場合でも球が小さくなりすぎないように
    // C++側で求めた最大スケールg_worldMaxScaleを掛ける
    const float l_worldRadius = l_meshletBounds.radius * g_worldMaxScale;
    
    return IsVisibleViewSpaceBoundingSphere(l_viewCenter.xyz, l_worldRadius);
}

#endif // MODEL_MESHLET_FRUSTUM_CULLING_HLSLI