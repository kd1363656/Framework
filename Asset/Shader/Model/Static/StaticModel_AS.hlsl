#include "StaticModel.hlsli"

// Frustum側面PlaneでBoundingSphere判定するときの半径補正を計算する
float CalculateStaticModelFrustumPlaneRadius(const float a_worldRadius, const float a_tanHalfFOV)
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
    const float l_horizontalPlaneRadius = CalculateStaticModelFrustumPlaneRadius(a_worldRadius, g_tanHalfFOVX);
    
    // Sphere全体が左外側、右外側にあるなら見えない。
    if (a_viewCenter.x + l_horizontalPlaneRadius < -l_halfViewWidth ||
        a_viewCenter.x - l_horizontalPlaneRadius >  l_halfViewWidth)
    {
        return false;
    }
    
    // View空間のz位置における、画面上端までの距離。
    // Zが大きいほど、みえる縦幅は広がる
    const float l_halfViewHeight      = l_frustumTestDepth * g_tanHalfFOVY;
    const float l_verticalPlaneRadius = CalculateStaticModelFrustumPlaneRadius(a_worldRadius, g_tanHalfFOVY);
    
    // Sphere全体が下外側、上外側にあるなら見えない
    if (a_viewCenter.y + l_verticalPlaneRadius < -l_halfViewHeight ||
        a_viewCenter.y - l_verticalPlaneRadius >  l_halfViewHeight)
    {
        return false;
    }
    
    return true;
}

// カメラがMeshletのBoundingSphere内に入っているか判定する。
bool IsCameraInsideStaticModelMeshletBoundingSphere(const ModelMeshletBounds a_meshletBounds)
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
bool IsVisibleStaticModelMeshletByFrustum(const uint a_meshletIndex)
{
    // MeshletごとのBoundingSphere情報を読む
    StructuredBuffer<ModelMeshletBounds> l_meshletBoundsBuffer = ResourceDescriptorHeap[g_meshletBoundsBufferSRVDescriptorIndex];

    const ModelMeshletBounds l_meshletBounds = l_meshletBoundsBuffer[a_meshletIndex];
    
    // カメラがMeshletのBoundingSphere内にいる場合、
    // NearPlaneやFrustum側面で誤カリングされやすいのでFrustum内として扱う
    if (IsCameraInsideStaticModelMeshletBoundingSphere(l_meshletBounds)) { return true; }
    
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

// 指定したMeshletがカメラから見て完全に裏向きかを判定する
// バックフェースコーンカリングはラスタライザーのCullBackを行っている
// しかし描画する前から決めれる、ラスタライザーは描画し終わってからカリングするといった都合上
// バックフェースコーンカリングを増幅シェーダーで行ったほうが効率的である
bool IsBackfaceStaticModelMeshletByCone(const uint a_meshletIndex)
{
    StructuredBuffer<ModelMeshletBounds> l_meshletBoundsBuffer = ResourceDescriptorHeap[g_meshletBoundsBufferSRVDescriptorIndex];
    
    const ModelMeshletBounds l_meshletBounds = l_meshletBoundsBuffer[a_meshletIndex];
    
    // coneApexはLocal空間の位置
    // 位置なのでw = 1.0FとしてWorld空間へ変換し、さらにView空間へ変換する
    const float4 l_localConeApex = float4(l_meshletBounds.coneApex, k_modelPositionElementW);
    const float4 l_worldConeApex = mul   (l_localConeApex,          g_worldMatrix);
    const float4 l_viewConeApex  = mul   (l_worldConeApex,          g_viewMatrix);
    
    // coneAxisはLocal空間の方向
    // 方向なのでw = 0.0F
    // さらに法線系の向きなので、非均一スケールを考えて逆行列の転置でWorld空間へ変換する
    const float4 l_localConeAxis = float4(l_meshletBounds.coneAxis, k_modelDirectionElementW);
    const float4 l_worldConeAxis = mul   (l_localConeAxis,          g_worldInverseTransposeMatrix);

    // View行列はカメラ空間への変換
    // 方向なのでw = 0.0Fとして平行移動の影響を受けないようにする
    const float4 l_viewConeAxis = mul(float4(normalize(l_worldConeAxis.xyz), k_modelDirectionElementW), g_viewMatrix);

    // ビュー行列ではカメラが(0.0F, 0.0F, 0.0F)原点になるので
    // わざわざコーン位置を引く必要がないためLengthでベクトルを算出
    const float l_viewConeApexLength = length(l_viewConeApex.xyz);
    const float l_viewConeAxisLength = length(l_viewConeAxis.xyz);

    // normalize(0)をよける
    // この場合は安全側に倒して見える可能性ありとする
    if (l_viewConeApexLength <= k_modelMeshletCullingEpsilon ||
        l_viewConeAxisLength <= k_modelMeshletCullingEpsilon)
    {
        return false;
    }
    
    // View空間ではカメラ位置が原点なので、
    // normalize(cone_apex - camera_position)はnormalize(l_viewConeApex.xyz)でよい
    // 位置からベクトルと長さを持つLengthを割ることで方向ベクトルを取得する
    const float3 l_cameraToConeApexDirection = l_viewConeApex.xyz / l_viewConeApexLength;
    const float3 l_viewConeAxisDirection     = l_viewConeAxis.xyz / l_viewConeAxisLength;
    
    // meshoptimizerのPerspective用判定式
    // dot(normalize(cone_apex - camera_position), cone_axis) >= cone_cutoff
    // これを満たすなら、このMeshlet内の三角形群はカメラから見て裏向きと判断できる
    return dot(l_cameraToConeApexDirection, l_viewConeAxisDirection) >= l_meshletBounds.coneCutoff;
}

// FrustumCulling / BackfaceConeCullingを実行してどちらの条件にも一致しなければ
// 描画するようにする
bool ShouldDispatchStaticModelMeshlet(const uint a_meshletIndex)
{
    if (!IsVisibleStaticModelMeshletByFrustum(a_meshletIndex) ||
         IsBackfaceStaticModelMeshletByCone  (a_meshletIndex))
    {
        return false;
    }

    return true;
}

// Model共通のAmplificationShader
[numthreads(k_modelAmplificationShaderThreadCountX, k_modelAmplificationShaderThreadCountY, k_modelAmplificationShaderThreadCountZ)]
void main(uint3 a_groupID : SV_GroupID)
{
    ModelAmplificationPayload l_payload;
    
    // CPU側が(meshletCount, 1, 1)を呼びため、
    // ASのa_groupID.xはそのままMeshletIndexとして使える。
    const uint l_meshletIndex = a_groupID.x;
    
    // Frustum外またはカメラから見て完全に裏向きなら、このMeshletのMeshShaderを起動しない
    const bool l_isVisible = ShouldDispatchStaticModelMeshlet(l_meshletIndex);
    
    // 見えているMeshletだけMeshShaderにMeshletIndexを渡す
    l_payload.meshletIndex = l_meshletIndex;
    
    const uint l_dispatchMeshGroupCountX = l_isVisible ? k_modelAmplificationDispatchMeshGroupCountX : k_modelAmplificationDispatchMeshCulledGroupCountX;
    
    DispatchMesh(l_dispatchMeshGroupCountX, k_modelAmplificationDispatchMeshGroupCountY, k_modelAmplificationDispatchMeshGroupCountZ, l_payload);
}