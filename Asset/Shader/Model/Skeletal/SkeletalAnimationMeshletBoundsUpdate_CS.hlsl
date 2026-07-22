#include "../ModelMeshlet.hlsli"
#include "SkeletalAnimationModel.hlsli"

cbuffer CBSkeletalAnimationMeshletBoundsUpdatePerObject : register(b0)
{
    uint g_vertexBufferSRVDescriptorIndex;
    uint g_meshletBufferSRVDescriptorIndex;
    uint g_uniqueVertexIndexBufferSRVDescriptorIndex;
    uint g_meshletBoundsBufferUAVDescriptorIndex;
}

static const float k_boundingSphereCenterAverageScale = 0.5F;

static const uint k_meshletBoundsUpdateThreadCountX = 64U;
static const uint k_meshletBoundsUpdateThreadCountY = 1U;
static const uint k_meshletBoundsUpdateThreadCountZ = 1U;

static const uint k_reductionPairElementCount = 2U;

static const uint k_firstThreadIndex = 0U;

static const uint k_emptyMeshletVertexCount = 0U;

static const uint k_lastElementIndexOffsetFromCount = 1U;

groupshared float3 g_meshletPositionList       [k_meshletBoundsUpdateThreadCountX];
groupshared float3 g_meshletMinimumPositionList[k_meshletBoundsUpdateThreadCountX];
groupshared float3 g_meshletMaximumPositionList[k_meshletBoundsUpdateThreadCountX];
groupshared float  g_meshletRadiusSquaredList  [k_meshletBoundsUpdateThreadCountX];

[numthreads(k_meshletBoundsUpdateThreadCountX, k_meshletBoundsUpdateThreadCountY, k_meshletBoundsUpdateThreadCountZ)]
void main(const uint3 a_groupID          : SV_GroupID,
          const uint  a_groupThreadIndex : SV_GroupIndex)
{
    const uint l_meshletIndex = a_groupID.x;
    
    
    StructuredBuffer  <SkeletalAnimationSkinnedVertex> l_vertexBuffer            = ResourceDescriptorHeap[g_vertexBufferSRVDescriptorIndex];
    StructuredBuffer  <ModelMeshlet>                   l_meshletBuffer           = ResourceDescriptorHeap[g_meshletBufferSRVDescriptorIndex];
    StructuredBuffer  <uint>                           l_uniqueVertexIndexBuffer = ResourceDescriptorHeap[g_uniqueVertexIndexBufferSRVDescriptorIndex];
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
            const ModelMeshletBounds l_emptyMeshletBounds = (ModelMeshletBounds) 0;

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
    g_meshletPositionList       [a_groupThreadIndex] = l_position;
    g_meshletMinimumPositionList[a_groupThreadIndex] = l_position;
    g_meshletMaximumPositionList[a_groupThreadIndex] = l_position;
    
    // 全Threadが位置を書き込むまでAABBのReductionを開始しない
    GroupMemoryBarrierWithGroupSync();
    
    // Thread数を半分ずつ減らしながら
    // Meshlet内の前頂点を囲むAABBを求める
    // 64個の結果を32,16,8,4,2,1個へ集約する
    for (uint l_aabbReductionStride = k_meshletBoundsUpdateThreadCountX / k_reductionPairElementCount; l_aabbReductionStride > k_firstThreadIndex; l_aabbReductionStride /= k_reductionPairElementCount)
    {
        if (a_groupThreadIndex < l_aabbReductionStride)
        {
            const uint l_compareThreadIndex = a_groupThreadIndex + l_aabbReductionStride;
            
            g_meshletMinimumPositionList[a_groupThreadIndex] = min(g_meshletMinimumPositionList[a_groupThreadIndex], g_meshletMinimumPositionList[l_compareThreadIndex]);
            g_meshletMaximumPositionList[a_groupThreadIndex] = max(g_meshletMaximumPositionList[a_groupThreadIndex], g_meshletMaximumPositionList[l_compareThreadIndex]);
        }
        
        // 現在のReduction段階が完了する前に、次の段階に進まないように同期する
        GroupMemoryBarrierWithGroupSync();
    }
    
    // Reduction完了後、先頭要素にMeshlet全体の最小位置と最大一が格納される
    const float3 l_minimumMeshletPosition = g_meshletMinimumPositionList[k_firstThreadIndex];
    const float3 l_maximumMeshletPosition = g_meshletMaximumPositionList[k_firstThreadIndex];
    
    // AABBの中央をBoundigSphereの中心として使用する
    // meshoptimizerが作る最小Sphereとは異なるが
    // 現在Poseの前兆店を安全に囲むSphereになる
    const float3 l_boundingSphereCenter = (l_minimumMeshletPosition + l_maximumMeshletPosition) * k_boundingSphereCenterAverageScale;
    
    // 各Threadが担当する頂点について
    // BoundingSphere中心からの距離の二乗を計算する
    // 頂点数を超えたThreadは最後の中点を複製しているため、
    // 距離をそのまま計算しても最大値の結果は変わらない
    const float3 l_boundingSphereCenterToVertex = g_meshletPositionList[a_groupThreadIndex] - l_boundingSphereCenter;
    
    g_meshletRadiusSquaredList[a_groupThreadIndex] = dot(l_boundingSphereCenterToVertex, l_boundingSphereCenterToVertex);
    
    // 全Threadが距離の二乗を書き込むまで
    // 半径のReductionを開始しない
    GroupMemoryBarrierWithGroupSync();
    
    // BoundingSphere中心から最も遠い頂点までの距離の二乗を求める
    for (uint l_radiusReductionStride = k_meshletBoundsUpdateThreadCountX / k_reductionPairElementCount; l_radiusReductionStride > k_firstThreadIndex; l_radiusReductionStride /= k_reductionPairElementCount)
    {
        if (a_groupThreadIndex < l_radiusReductionStride)
        {
            const uint l_compareThreadIndex = a_groupThreadIndex + l_radiusReductionStride;
            
            g_meshletRadiusSquaredList[a_groupThreadIndex] = max(g_meshletRadiusSquaredList[a_groupThreadIndex], g_meshletRadiusSquaredList[l_compareThreadIndex]);
        }
        
        GroupMemoryBarrierWithGroupSync();
    }
    
    // Group内の最初のThreadだけが、
    // 完成したBoundsをUAVへ書き込む
    if (a_groupThreadIndex == k_firstThreadIndex)
    {
        ModelMeshletBounds l_meshletBounds = (ModelMeshletBounds) 0;
        
        l_meshletBounds.center = l_boundingSphereCenter;
        
        // 距離の二乗から実際の半径を求める
        // 境界付近の浮動小数点誤差でSphereが小さくなり、
        // 見えているMeshletが誤って消えないよう、
        // Meshlet Culling用の許容誤差を加える
        l_meshletBounds.radius = sqrt(g_meshletRadiusSquaredList[k_firstThreadIndex]) + k_modelMeshletCullingEpsilon;

        l_meshletBoundsBuffer[l_meshletIndex] = l_meshletBounds;
    }
}