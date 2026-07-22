#include "StaticModel.hlsli"
#include "../ModelMeshletFrustumCulling.hlsli"

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
    if (!IsVisibleModelMeshletByFrustum(a_meshletIndex) ||
         IsBackfaceStaticModelMeshletByCone(a_meshletIndex))
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