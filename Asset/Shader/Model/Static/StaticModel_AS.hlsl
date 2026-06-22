#include "StaticModel.hlsli"

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

    // View空間のZ位置における、画面右端までの距離
    // Zが大きいほど、見える横幅は広がる
    const float l_halfViewWidth = a_viewCenter.z * g_tanHalfFOVX;
    
    // Sphere全体が左外側、右外側にあるなら見えない。
    if (a_viewCenter.x + a_worldRadius < -l_halfViewWidth ||
        a_viewCenter.x - a_worldRadius >  l_halfViewWidth)
    {
        return false;
    }
    
    // View空間のz位置における、画面上端までの距離。
    // Zが大きいほど、みえる縦幅は広がる
    const float l_halfViewHeight = a_viewCenter.z * g_tanHalfFOVY;
    
    // Sphere全体が下外側、上外側にあるなら見えない
    if (a_viewCenter.y + a_worldRadius < -l_halfViewHeight ||
        a_viewCenter.y - a_worldRadius > l_halfViewHeight)
    {
        return false;
    }
    
    return true;
}

// 指定したMeshletがFrustum内にあるか判定する
bool IsVisibleStaticModelMeshletByFrustum(const uint a_meshletIndex)
{
    // MeshletごとのBoundingSphere情報を読む
    StructuredBuffer<ModelMeshletBounds> l_meshletBoundsBuffer = ResourceDescriptorHeap[g_meshletBoundsBufferSRVDescriptorIndex];

    const ModelMeshletBounds l_meshletBounds = l_meshletBoundsBuffer[a_meshletIndex];
    
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

// Model共通のAmplificationShader
[numthreads(k_modelAmplificationShaderThreadCountX, k_modelAmplificationShaderThreadCountY, k_modelAmplificationShaderThreadCountZ)]
void main(uint3 a_groupID : SV_GroupID)
{
    ModelAmplificationPayload l_payload;
    
    // CPU側が(meshletCount, 1, 1)を呼びため、
    // ASのa_groupID.xはそのままMeshletIndexとして使える。
    const uint l_meshletIndex = a_groupID.x;
    
    // Frustum外なら、このMeshletのMeshShaderを起動しない
    // ここでreturnすると、このASグループは何も描画せずに終了する
    const bool l_isVisible = IsVisibleStaticModelMeshletByFrustum(l_meshletIndex);
    
    // 見えているMeshletだけMeshSahderにMeshletIndexを渡す
    l_payload.meshletIndex = l_meshletIndex;
    
    const uint l_dispatchMeshGroupCountX = l_isVisible ? k_modelAmplificationDispatchMeshGroupCountX : k_modelAmplificationDispatchMeshCulledGroupCountX;
    
    DispatchMesh(l_dispatchMeshGroupCountX, k_modelAmplificationDispatchMeshGroupCountY, k_modelAmplificationDispatchMeshGroupCountZ, l_payload);
}