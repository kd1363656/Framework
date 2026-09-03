#include "../ModelMeshlet.hlsli"
#include "SkeletalAnimationModel.hlsli"

cbuffer CBSkeletalAnimationMeshletBoundsUpdatePerObject : register(b0)
{
    uint g_vertexBufferSRVDescriptorIndex;
    uint g_meshletBufferSRVDescriptorIndex;
    uint g_uniqueVertexIndexBufferSRVDescriptorIndex;
    uint g_primitiveIndexBufferSRVDescriptorIndex;
    uint g_meshletBoundsBufferUAVDescriptorIndex;
}

static const float k_boundingSphereCenterAverageScale = 0.5F;

static const float k_meshletConeMAXNormalDot       = 1.0F;
static const float k_meshletConeMINUsefulNormalDot = 0.1F;

static const float k_triangleNormalLengthSquaredEpsilon = k_modelMeshletCullingEpsilon * k_modelMeshletCullingEpsilon;

static const uint k_meshletBoundsUpdateThreadCountX = 64U;
static const uint k_meshletBoundsUpdateThreadCountY = 1U;
static const uint k_meshletBoundsUpdateThreadCountZ = 1U;

static const uint k_reductionPairElementCount = 2U;

static const uint k_firstThreadIndex = 0U;

static const uint k_emptyMeshletVertexCount = 0U;

static const uint k_lastElementIndexOffsetFromCount = 1U;

static const uint k_meshletBoundsUpdateSecondTriangleIndexOffset = k_meshletBoundsUpdateThreadCountX;

groupshared float3 g_meshletPositionList     [k_meshletBoundsUpdateThreadCountX];
groupshared float3 g_meshletMINPositionList  [k_meshletBoundsUpdateThreadCountX];
groupshared float3 g_meshletMAXPositionList  [k_meshletBoundsUpdateThreadCountX];
groupshared float3 g_meshletConeAxisSumList  [k_meshletBoundsUpdateThreadCountX];
groupshared float  g_meshletRadiusSquaredList[k_meshletBoundsUpdateThreadCountX];
groupshared float  g_meshletConeMINDotList   [k_meshletBoundsUpdateThreadCountX];

float3 CalculateMeshletTriangleNormal(const uint a_packedPrimitiveIndex, const uint a_meshletVertexCount)
{
    // 失敗しても未初期化値は返さない
    float3 l_triangleNormal = (float3) 0;
    
    // uint一個にPackされた三個の
    // Meshlet内VertexIndexを展開する
    const uint3 l_meshletVertexIndexList = DecodeModelPackedPrimitiveIndex(a_packedPrimitiveIndex);

    // 壊れたMeshletDataによって
    // GroupSharedMemoryの範囲外を参照しないようにする
    if (any(l_meshletVertexIndexList >= a_meshletVertexCount)) { return l_triangleNormal; }

    const float3 l_firstPosition  = g_meshletPositionList[l_meshletVertexIndexList.x];
    const float3 l_secondPosition = g_meshletPositionList[l_meshletVertexIndexList.y];
    const float3 l_thirdPosition  = g_meshletPositionList[l_meshletVertexIndexList.z];
    
    // Primitive Indexの頂点順を維持した二本の辺を作る。
    // 頂点順を逆にするとNormalも逆向きになり、
    // Backface判定が壊れるため変更しない。
    const float3 l_firstEdge  = l_secondPosition - l_firstPosition;
    const float3 l_secondEdge = l_thirdPosition  - l_firstPosition;
    
    const float3 l_unnormalizedTriangleNormal  = cross(l_firstEdge,                  l_secondEdge);
    const float  l_triangleNormalLengthSquared = dot  (l_unnormalizedTriangleNormal, l_unnormalizedTriangleNormal);

    // 退化TriangleはRasterizerでも表示されないため、
    // NormalConeの計算対象から除外する
    if (l_triangleNormalLengthSquared <= k_triangleNormalLengthSquaredEpsilon) { return l_triangleNormal; }

    const float l_inverseTriangleNormalLength = rsqrt(l_triangleNormalLengthSquared);
    
    // Lengthを別に計算して除算せず
    // rsqrtを使って直接正規化する
    l_triangleNormal = l_unnormalizedTriangleNormal * l_inverseTriangleNormalLength;
    
    return l_triangleNormal;
}

[numthreads(k_meshletBoundsUpdateThreadCountX, k_meshletBoundsUpdateThreadCountY, k_meshletBoundsUpdateThreadCountZ)]
void main(const uint3 a_groupID          : SV_GroupID,
          const uint  a_groupThreadIndex : SV_GroupIndex)
{
    const uint l_meshletIndex = a_groupID.x;
    
    StructuredBuffer  <SkeletalAnimationSkinnedVertex> l_vertexBuffer            = ResourceDescriptorHeap[g_vertexBufferSRVDescriptorIndex];
    StructuredBuffer  <ModelMeshlet>                   l_meshletBuffer           = ResourceDescriptorHeap[g_meshletBufferSRVDescriptorIndex];
    StructuredBuffer  <uint>                           l_uniqueVertexIndexBuffer = ResourceDescriptorHeap[g_uniqueVertexIndexBufferSRVDescriptorIndex];
    StructuredBuffer  <uint>                           l_primitiveIndexBuffer    = ResourceDescriptorHeap[g_primitiveIndexBufferSRVDescriptorIndex];
    RWStructuredBuffer<ModelMeshletBounds>             l_meshletBoundsBuffer     = ResourceDescriptorHeap[g_meshletBoundsBufferUAVDescriptorIndex];
    
    const ModelMeshlet l_meshlet = l_meshletBuffer[l_meshletIndex];
    
    // 通常のMeshletは一つ以上の頂点を持つ
    // 壊れたデータなどによっては頂点数がゼロだった場合に
    // UniqueVertexIndexBufferの範囲外を参照しないようにする
    if (l_meshlet.vertexCount == k_emptyMeshletVertexCount)
    {
        if (a_groupThreadIndex == k_firstThreadIndex)
        {
            // Cone情報を含め、すべてゼロで初期化する。
            // 空Meshletなので半径ゼロのBoundsを書き込み、
            // 未初期化データがASから読み取られることを防ぐ   
            ModelMeshletBounds l_emptyMeshletBounds = (ModelMeshletBounds) 0;

            l_emptyMeshletBounds.coneCutoff       = k_modelDisabledMeshletConeCutoff;
            l_meshletBoundsBuffer[l_meshletIndex] = l_emptyMeshletBounds;
        }
        
        return;
    }
    
    const uint   l_lastMeshletVertexIndex = l_meshlet.vertexCount - k_lastElementIndexOffsetFromCount;
    const uint   l_meshletVertexIndex     = min(a_groupThreadIndex, l_lastMeshletVertexIndex);
    const uint   l_uniqueVertexIndex      = l_meshlet.vertexOffset + l_meshletVertexIndex;
    const uint   l_modelVertexIndex       = l_uniqueVertexIndexBuffer[l_uniqueVertexIndex];
    const float3 l_position               = l_vertexBuffer           [l_modelVertexIndex].position;
    
    // 各Threadが読み込んだ頂点位置をSharedMemoryへ格納する
    g_meshletPositionList   [a_groupThreadIndex] = l_position;
    g_meshletMINPositionList[a_groupThreadIndex] = l_position;
    g_meshletMAXPositionList[a_groupThreadIndex] = l_position;
    
    // 全Threadが位置を書き込むまでAABBのReductionを開始しない
    GroupMemoryBarrierWithGroupSync();
    
    // 全Threadは最大二個のTriangleを担当する
    const uint l_firstMeshletTriangleIndex  = a_groupThreadIndex;
    const uint l_secondMeshletTriangleIndex = a_groupThreadIndex + k_meshletBoundsUpdateSecondTriangleIndexOffset;
    
    float3 l_firstTriangleNormal  = (float3) 0;
    float3 l_secondTriangleNormal = (float3) 0;
    
    if (l_firstMeshletTriangleIndex < l_meshlet.triangleCount)
    {
        const uint l_firstPackedPrimitiveIndex = l_primitiveIndexBuffer[l_meshlet.triangleOffset + l_firstMeshletTriangleIndex];
        
        l_firstTriangleNormal = CalculateMeshletTriangleNormal(l_firstPackedPrimitiveIndex, l_meshlet.vertexCount);
    }
    
    if (l_secondMeshletTriangleIndex < l_meshlet.triangleCount)
    {
        const uint l_secondPackedPrimitiveIndex = l_primitiveIndexBuffer[l_meshlet.triangleOffset + l_secondMeshletTriangleIndex];
        
        l_secondTriangleNormal = CalculateMeshletTriangleNormal(l_secondPackedPrimitiveIndex, l_meshlet.vertexCount);
    }
    
    // 正常に計算されたTriangle Normalは正規化済みなので、
    // LengthSquaredはほぼOneになる。
    // Zero Vectorの場合はLengthSquaredがZeroになるため、
    // Triangle Normalが有効か判定できる。
    const bool l_isFirstTriangleNormalValid  = dot(l_firstTriangleNormal, l_firstTriangleNormal)   > k_triangleNormalLengthSquaredEpsilon;
    const bool l_isSecondTriangleNormalValid = dot(l_secondTriangleNormal, l_secondTriangleNormal) > k_triangleNormalLengthSquaredEpsilon;
    
    // 各Threadが担当した有効Triangle Normalを合計する。
    // 無効なTriangleはZero Vectorなので、
    // そのまま加算しても結果へ影響しない。
    const float3 l_triangleNormalSum = l_firstTriangleNormal + l_secondTriangleNormal;
    
    g_meshletConeAxisSumList[a_groupThreadIndex] = l_triangleNormalSum;
    
    // 全ThreadがNormal合計を書き込んでから、
    // Cone AxisのReductionを開始する。
    GroupMemoryBarrierWithGroupSync();
    
    // Thread数を半分ずつ減らしながら
    // Meshlet内の全頂点を囲むAABBを求める
    // 64個の結果を32,16,8,4,2,1個へ集約する
    for (uint l_aabbReductionStride = k_meshletBoundsUpdateThreadCountX / k_reductionPairElementCount; l_aabbReductionStride > k_firstThreadIndex; l_aabbReductionStride /= k_reductionPairElementCount)
    {
        if (a_groupThreadIndex < l_aabbReductionStride)
        {
            const uint l_compareThreadIndex = a_groupThreadIndex + l_aabbReductionStride;
            
            g_meshletMINPositionList[a_groupThreadIndex] = min(g_meshletMINPositionList[a_groupThreadIndex], g_meshletMINPositionList[l_compareThreadIndex]);
            g_meshletMAXPositionList[a_groupThreadIndex] = max(g_meshletMAXPositionList[a_groupThreadIndex], g_meshletMAXPositionList[l_compareThreadIndex]);
            
            // AABBと同じReduction段階を利用して、
            // 全Triangle Normalの合計も求める。
            // Cone専用のReductionを別に実行しないため、
            // 追加の同期回数を抑えられる。
            g_meshletConeAxisSumList[a_groupThreadIndex] += g_meshletConeAxisSumList[l_compareThreadIndex];
        }
        
        // 現在のReduction段階が完了する前に、次の段階に進まないように同期する
        GroupMemoryBarrierWithGroupSync();
    }
    
    // Reduction完了後、先頭要素にMeshlet全体の最小位置と最大位置が格納される
    const float3 l_minMeshletPosition = g_meshletMINPositionList[k_firstThreadIndex];
    const float3 l_maxMeshletPosition = g_meshletMAXPositionList[k_firstThreadIndex];
    const float3 l_meshletConeAxisSum = g_meshletConeAxisSumList[k_firstThreadIndex];
    
    // 合計Normalが正規化できる長さを持つか確認する。
    // 表裏が反対のNormal同士で相殺された場合や、
    // 有効なTriangleが存在しない場合はZero付近になる。
    const float l_meshletConeAxisLengthSquared =
    dot
    (
        l_meshletConeAxisSum,
        l_meshletConeAxisSum
    );
    
    const bool l_hasValidMeshletConeAxis = l_meshletConeAxisLengthSquared > k_triangleNormalLengthSquaredEpsilon;
    
    // 無効だった場合はZeroVerctorのままにする
    float3 l_meshletConeAxis = (float3) 0;
    
    // rsqrt(Zero)を避けるため、
    // 有効な長さを持つ場合だけ正規化する。
    if (l_hasValidMeshletConeAxis)
    {
        const float l_inverseMeshletConeAxisLength = rsqrt(l_meshletConeAxisLengthSquared);
    
        l_meshletConeAxis = l_meshletConeAxisSum * l_inverseMeshletConeAxisLength;
    }
    
    // AABBの中央をBoundingSphereの中心として使用する
    // meshoptimizerが作る最小Sphereとは異なるが
    // 現在Poseの全頂点を安全に囲むSphereになる
    const float3 l_boundingSphereCenter = (l_minMeshletPosition + l_maxMeshletPosition) * k_boundingSphereCenterAverageScale;
    
    // 各Threadが担当する頂点について
    // BoundingSphere中心からの距離の二乗を計算する
    // 頂点数を超えたThreadは最後の頂点を複製しているため、
    // 距離をそのまま計算しても最大値の結果は変わらない
    const float3 l_boundingSphereCenterToVertex = g_meshletPositionList[a_groupThreadIndex] - l_boundingSphereCenter;
    
    g_meshletRadiusSquaredList[a_groupThreadIndex] = dot(l_boundingSphereCenterToVertex, l_boundingSphereCenterToVertex);
    
    float l_meshletConeMINDot = k_meshletConeMAXNormalDot;
    
    // Cone Axisとの内積を取得する。
    if (l_hasValidMeshletConeAxis && l_isFirstTriangleNormalValid)
    {
        l_meshletConeMINDot = min(l_meshletConeMINDot, dot(l_meshletConeAxis, l_firstTriangleNormal) );
    }
    
    if (l_hasValidMeshletConeAxis && l_isSecondTriangleNormalValid)
    {
        l_meshletConeMINDot = min(l_meshletConeMINDot, dot(l_meshletConeAxis, l_secondTriangleNormal));
    }
    
    // このThreadが担当したTriangleのうち、
    // ConeAxisから最も離れているNormalの内積を保存する。
    g_meshletConeMINDotList[a_groupThreadIndex] = l_meshletConeMINDot;
    
    // 全ThreadがRadiusSquaredとConeMINDotを書き込むまで、
    // 二回目のReductionを開始しない。
    GroupMemoryBarrierWithGroupSync();
    
    // BoundingSphere中心から最も遠い頂点までの距離の二乗を求める
    for (uint l_radiusReductionStride = k_meshletBoundsUpdateThreadCountX / k_reductionPairElementCount; l_radiusReductionStride > k_firstThreadIndex; l_radiusReductionStride /= k_reductionPairElementCount)
    {
        if (a_groupThreadIndex < l_radiusReductionStride)
        {
            const uint l_compareThreadIndex = a_groupThreadIndex + l_radiusReductionStride;
            
            g_meshletRadiusSquaredList[a_groupThreadIndex] = max(g_meshletRadiusSquaredList[a_groupThreadIndex], g_meshletRadiusSquaredList[l_compareThreadIndex]);
            
            // Radiusの最大値を求めるReductionと同じ段階で、
            // Cone AxisとTriangle Normalの最小内積も求める。
            // Cone専用Reductionを追加しないため、
            // 新しい同期ループは必要ない。
            g_meshletConeMINDotList[a_groupThreadIndex] = min(g_meshletConeMINDotList[a_groupThreadIndex], g_meshletConeMINDotList[l_compareThreadIndex]);
        }
        
        GroupMemoryBarrierWithGroupSync();
    }
    
    // Group内の最初のThreadだけが、
    // 完成したBoundsをUAVへ書き込む
    if (a_groupThreadIndex == k_firstThreadIndex)
    {
        ModelMeshletBounds l_meshletBounds = (ModelMeshletBounds) 0;
        
        l_meshletBounds.center = l_boundingSphereCenter;
        
        // Reductionで求めた最大距離の二乗から半径を計算する。
        // 境界付近の浮動小数点誤差によってSphereが小さくなり、
        // 見えるMeshletを誤ってカリングしないようEpsilonを加える
        l_meshletBounds.radius = sqrt(g_meshletRadiusSquaredList[k_firstThreadIndex]) + k_modelMeshletCullingEpsilon;

        // 初期状態ではBackface Cone Cullingを無効にする
        //
        // 有効なConeを生成できた場合だけ、
        // この後でconeAxisとconeCutoffを上書きする
        l_meshletBounds.coneCutoff = k_modelDisabledMeshletConeCutoff;
        
        // Reduction後の先頭要素には、
        // Meshlet内すべてのTriangle Normalについて求めた
        // Cone Axisとの最小内積が格納されている
        const float l_meshletConeMINNormalDot = g_meshletConeMINDotList[k_firstThreadIndex];
        
        // Axisが正常に作成でき、さらにNormal群が十分狭い場合だけ
        // Backface Cone Culling用の情報を有効化する
        // Minimum Dotが小さい場合は、
        // Triangle Normalが広い範囲へ分散していることを表す
        if (l_hasValidMeshletConeAxis &&
            l_meshletConeMINNormalDot > k_meshletConeMINUsefulNormalDot)
        {
            // Minimum Dotは、Cone Axisと最も離れたNormalとのcos値
            // Backface判定で使用するCutoffはsin値なので、
            // sqrt(One - MinimumDotSquared)で求める
            const float l_meshletConeCutoffSquared = saturate(k_meshletConeMAXNormalDot - l_meshletConeMINNormalDot * l_meshletConeMINNormalDot);
            
            l_meshletBounds.coneAxis = l_meshletConeAxis;
            
            // Epsilonを加えてCutoffを少し大きくする。
            // Cutoffが大きいほどカリング条件が厳しくなるため、
            // 境界付近で見えているMeshletが消えにくい安全側になる
            l_meshletBounds.coneCutoff = min(k_modelDisabledMeshletConeCutoff, sqrt(l_meshletConeCutoffSquared) + k_modelMeshletCullingEpsilon);
        }
        
        // 今回生成する動的Coneは、次段階で
        // Bounding Sphereを使用する判定式へ接続する
        // そのためconeApexは使用せず、
        // Zero初期化された値のまま保存する
        l_meshletBoundsBuffer[l_meshletIndex] = l_meshletBounds;
    }
}