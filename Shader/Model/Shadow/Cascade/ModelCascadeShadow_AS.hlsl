#include "ModelCascadeShadowMeshletCulling.hlsli"

groupshared ModelAmplificationPayload g_modelAmplificationPayload;
groupshared uint                      g_modelVisibleMeshletCount;

[numthreads(k_modelAmplificationShaderThreadCountX, k_modelAmplificationShaderThreadCountY, k_modelAmplificationShaderThreadCountZ)]
void main(const uint3 a_dispatchThreadID : SV_DispatchThreadID,
          const uint  a_groupThreadIndex : SV_GroupIndex)
{
    // Group内の代表Threadだけが可視Meshlet数を初期化する
    if (a_groupThreadIndex == k_modelAmplificationLeaderThreadIndex)
    {
        g_modelVisibleMeshletCount = k_modelAmplificationInitialVisibleMeshletCount;
    }
    
    // 初期化が完了するまでGroup内の全Threadを待機させる
    GroupMemoryBarrierWithGroupSync();
    
    // 1つのthreadが1つのMeshletを担当する
    // a_dispatchThreadID.xは「描画するLODの中で何番目のMeshletか」を表すため、
    // LODの先頭番号を足して、全LODを連結したBuffer全体での番号へ変換する
    const uint l_lodMeshletIndex = a_dispatchThreadID.x;
    const uint l_meshletIndex    = g_meshletOffset + l_lodMeshletIndex;
          bool l_shouldDispatch  = false;

    // 最後のASGroupには実際のMeshlet数を超えるThreadが含まれる
    // 範囲内のThreadだけがMeshletBoundsを参照する
    if (l_lodMeshletIndex < g_meshletCount)
    {
        // GPUスキニングするMeshは、Meshletの範囲(元の姿勢で計算した物)が動いた形と合わないため、
        // Meshlet単位のカリングをせずに全て描画する(Mesh単位のカリングは動いた形を囲む球で済ませている)
        l_shouldDispatch = g_isGPUSkinning != k_modelFalseValue ? true : ShouldDispatchModelCascadeShadowMeshlet(l_meshletIndex);
    }
    
    if (l_shouldDispatch)
    {
        uint l_payloadMeshletIndex = k_modelAmplificationInitialVisibleMeshletCount;
        
        // 可視Meshletを書き込むPayload配列位置を確保する
        InterlockedAdd(g_modelVisibleMeshletCount, k_modelAmplificationVisibleMeshletCountIncrement, l_payloadMeshletIndex);
        
        g_modelAmplificationPayload.meshletIndexList[l_payloadMeshletIndex] = l_meshletIndex;
    }
    
    DispatchMesh(g_modelVisibleMeshletCount,
                 k_modelAmplificationDispatchMeshGroupCountY,
                 k_modelAmplificationDispatchMeshGroupCountZ,
                 g_modelAmplificationPayload);
}