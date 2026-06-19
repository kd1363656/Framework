struct ModelMeshlet
{
    uint vertexOffset;
    uint vertexCount;
    uint triangleOffset;
    uint triangleCount;
};

// Meshlet1個分のカリング用境界情報
struct ModelMeshletBounds
{
    float3 center;
    float  radius;
    
    float3 coneApex;
    float  coneCutoff;
    
    float3 coneAxis;
    float  padding;
};

// AmplificationShaderからMeshShaderへ渡す値情報
// 「描画するMeshlet番号」だけを渡す
struct ModelMeshPayload
{
    uint meshletIndex;
};

// StaticModelのMeshShaderからPixelShaderへ渡すSceneColor描画用出力
struct ModelMeshOutput
{
    float4 position      : SV_Position;
    float3 worldPosition : POSITION0;
    float3 worldNormal   : NORMAL0;
    float4 worldTangent  : TANGENT0;
    float2 uv            : TEXCOORD0;
};

static const uint k_modelTriangleVertexCount = 3U;

static const uint k_modelMaxMeshletVertexCount    = 64U;
static const uint k_modelMaxMeshletPrimitiveCount = 126U;

static const uint k_modelAmplificationShaderThreadCountX = 1U;
static const uint k_modelAmplificationShaderThreadCountY = 1U;
static const uint k_modelAmplificationShaderThreadCountZ = 1U;

static const uint k_modelMeshShaderThreadCountX = 1U;
static const uint k_modelMeshShaderThreadCountY = 1U;
static const uint k_modelMeshShaderThreadCountZ = 1U;

static const uint k_modelSecondPrimitiveVertexOffset = 1U;
static const uint k_modelThirdPrimitiveVertexOffset  = 2U;

static const float k_modelPositionVectorElementW  = 1.0F;
static const float k_modelDirectionVectorElementW = 0.0F;

// meshoptimizerのConeCutoffは基本的に -1.0F ～ 1.0F の範囲。
// 1.0Fより大きい値なら無効扱いにする。
static const float k_modelInvalidConeCutoff = 1.0F;

// ModelのLocal座標をView座標へ変換する
// ASカリングでは、MEshletBounds.centerをView空間へ持っていくために使う。
float3 TransformModelLocalPositionToView(const matrix a_worldMatrix, const matrix a_viewMatrix, const float3 a_localPosition)
{
    const float4 l_worldPosition = mul(float4(a_localPosition, k_modelPositionVectorElementW), a_worldMatrix);
    
    return mul(l_worldPosition, a_viewMatrix).xyz;
}

// ModelのLocal方向ベクトルをview方向ベクトルへ変換する
float3 TransformModelLocalDirectionToView(const matrix a_worldMatrix, const matrix a_viewMatrix, const float3 a_localdirection)
{
    const float4 l_worldDirection = mul(float4(a_localdirection, k_modelDirectionVectorElementW), a_worldMatrix);
    
    return normalize(mul(l_worldDirection, a_viewMatrix).xyz);
}

// MeshletのBoundingSphereがViewFrustum外にあるか判定する。
// trueなら「完全にカメラ外名のっで描画しない」
bool IsOutsizeModelMeshletViewFrustum(const ModelMeshletBounds a_modelMeshletBounds,
                                      const matrix             a_worldMatrix,
                                      const matrix             a_viewMatrix,
                                      const float              a_worldMaxScale,
                                      const float              a_nearClip,
                                      const float              a_farClip,
                                      const float              a_tanHalfFOVX,
                                      const float              a_tanHalfFOVY)
{
    const float3 l_viewCenter  = TransformModelLocalPositionToView(a_worldMatrix, a_viewMatrix, a_modelMeshletBounds.center);
    const float  l_worldRadius = a_modelMeshletBounds.radius * a_worldMaxScale;
    
    if (l_viewCenter.z + l_worldRadius < a_nearClip) { return true; }
    if (l_viewCenter.z + l_worldRadius < a_nearClip) { return true; }
    
    const float l_limitX = l_viewCenter.z * a_tanHalfFOVX + l_worldRadius;
    const float l_limitY = l_viewCenter.z * a_tanHalfFOVY + l_worldRadius;

    if (l_viewCenter.x < -l_limitX || l_viewCenter.x > l_limitX) { return true; }
    if (l_viewCenter.y < -l_limitX || l_viewCenter.y > l_limitY) { return true; }
    
    return false;
}

// MeshletがBackfaceConeCullinで描画不要か判定する。
// trueならMeshlet全体が裏向きなので描画しない
bool IsCulledModelMeshletByBackfaceCone(const ModelMeshletBounds a_modelMeshletBounds, const matrix a_worldMatrix, const matrix a_viewMatrix)
{
    if (a_modelMeshletBounds.coneCutoff > k_modelInvalidConeCutoff) { return false; }
    
    const float3 l_viewConeApex = TransformModelLocalPositionToView (a_worldMatrix, a_viewMatrix, a_modelMeshletBounds.coneApex);
    const float3 l_viewConxAxis = TransformModelLocalDirectionToView(a_worldMatrix, a_viewMatrix, a_modelMeshletBounds.coneAxis);
    
    // View空間ではカメラ位置は原点
    // 原点からConceApexへ向かう方向がカメラからMeshlet方向になる
    const float3 l_viewDirectionFromCameraToConeApex = normalize(l_viewConeApex);

    return dot(l_viewDirectionFromCameraToConeApex, l_viewConxAxis) >= a_modelMeshletBounds.coneCutoff;

}

// Meshletを描画しなくてよいか判定する
bool IsCulledModelMeshlet(const ModelMeshletBounds a_modelMeshletBounds,
                          const matrix             a_worldMatrix,
                          const matrix             a_viewMatrix,
                          const float              a_worldMaxScale,
                          const float              a_nearClip,
                          const float              a_farClip,
                          const float              a_tanHalfFOVX,
                          const float              a_tanHalfFOVY)
{
    if (IsOutsizeModelMeshletViewFrustum(a_modelMeshletBounds,
                                         a_worldMatrix,
                                         a_viewMatrix,
                                         a_worldMaxScale,
                                         a_nearClip,
                                         a_farClip,
                                         a_tanHalfFOVX,
                                         a_tanHalfFOVY))
    {
        return true;
    }
    
    if (IsCulledModelMeshletByBackfaceCone(a_modelMeshletBounds, a_worldMatrix, a_viewMatrix))
    {
        return true;
    }

    return false;
}