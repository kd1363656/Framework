#include "../ModelMeshletCulling.hlsli"

groupshared ModelAmplificationPayload g_modelAmplificationPayload;
groupshared uint                      g_modelVisibleMeshletCount;

// Model共通のAmplificationShader
[numthreads(k_modelAmplificationShaderThreadCountX, k_modelAmplificationShaderThreadCountY, k_modelAmplificationShaderThreadCountZ)]
void main(const uint3 a_dispatchThreadID : SV_DispatchThreadID,
          const uint  a_groupThreadIndex : SV_GroupIndex)
{
    // Group内の代表Threadだけが可視Meshlet数を初期化する
    if (a_groupThreadIndex == k_modelAmplificationLeaderThreadIndex)
    {
        g_modelVisibleMeshletCount = k_modelAmplificationInitialVisibleMeshletCount;
    }
    
    // 他のthreadが初期化前の値へアクセスしないように、
    // Group内の全Threadをここで同期する
    GroupMemoryBarrierWithGroupSync();
    
    // SV_DispatchThreadID.xは、全てのAmplificationShaderGroupを通した連続ThreadIndex
    // 1つのThreadが1つのMeshletを担当するため、
    // この値は「描画するLODの中で何番目のMeshletか」を表す
    const uint l_lodMeshletIndex = a_dispatchThreadID.x;

    // 全LODのMeshletは1つのBufferへ連結されているため、
    // LODの先頭番号を足してBuffer全体での番号へ変換する
    const uint l_meshletIndex = g_meshletOffset + l_lodMeshletIndex;

    bool l_shouldDispatch = false;

    // 最後のAmplificationShaderGroupには、
    // 実際のMeshlet数を超えた余分なThreadが含まれる可能性がある
    // 範囲外のThreadはMeshletBoundsBufferへアクセスさせない
    if (l_lodMeshletIndex < g_meshletCount)
    {
        // GPUスキニングするMeshは、Meshletの範囲(元の姿勢で計算した物)が動いた形と合わないため、
        // Meshlet単位のカリングをせずに全て描画する(Mesh単位のカリングは動いた形を囲む球で済ませている)
        l_shouldDispatch = g_isGPUSkinning != 0U ? true : ShouldDispatchModelMeshlet(l_meshletIndex);
    }
    
    if (l_shouldDispatch)
    {
        uint l_payloadMeshletIndex = k_modelAmplificationInitialVisibleMeshletCount;
        
        // 可視Meshletを書き込む配列位置を一つ確保する
        // InterlockedAddの第三引数には加算前の値が入るため、
        // 可視Meshletは配列のZero番目から連続して格納される
        InterlockedAdd(g_modelVisibleMeshletCount, k_modelAmplificationVisibleMeshletCountIncrement, l_payloadMeshletIndex);
        
        g_modelAmplificationPayload.meshletIndexList[l_payloadMeshletIndex] = l_meshletIndex;
    }
    
    // DispatchMesh事態にGroupMemoryBarrierWithGroupSync相当の動機が含まれる
    // 可視判定を通過したMeshlet数だけ、子となるMeshShaderGroupを起動する
    // 可視Meshletが存在しなければ、GroupCountXは0となりMeshShaderは起動されない
    DispatchMesh(g_modelVisibleMeshletCount, 
                 k_modelAmplificationDispatchMeshGroupCountY, 
                 k_modelAmplificationDispatchMeshGroupCountZ, 
                 g_modelAmplificationPayload);
}