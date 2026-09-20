#include "../Model.hlsli"
#include "../ModelMeshletCulling.hlsli"

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
    
    // 初期化が完了するまで全threadを待機させる
    GroupMemoryBarrierWithGroupSync();
    
    // 1つのthreadが1つのMeshletを担当する
    const uint l_meshletIndex   = a_dispatchThreadID.x;
          bool l_shouldDispatch = false;
    
    // 最後のASGroupで範囲外となるThreadは、
    // 動的MeshletBoundsBufferへアクセスさせない
    if (l_meshletIndex < g_meshletCount)
    {
        l_shouldDispatch = ShouldDispatchModelMeshlet(l_meshletIndex);
    }
    
    if (l_shouldDispatch)
    {
        uint l_payloadMeshletIndex = k_modelAmplificationInitialVisibleMeshletCount;
        
        // 可視Meshletを書き込むPayload配列位置を確認する
        InterlockedAdd(g_modelVisibleMeshletCount, k_modelAmplificationVisibleMeshletCountIncrement, l_payloadMeshletIndex);
        
        g_modelAmplificationPayload.meshletIndexList[l_payloadMeshletIndex] = l_meshletIndex;
    }

    // 可視Meshlet数だけ子MeshShaderGroupを起動する
    DispatchMesh(g_modelVisibleMeshletCount,
                 k_modelAmplificationDispatchMeshGroupCountY,
                 k_modelAmplificationDispatchMeshGroupCountZ,
                 g_modelAmplificationPayload);
}