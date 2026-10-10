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

    // この描画のオブジェクト(行列)とメッシュ(バッファの番号・Meshletの数)を、テーブルから読む
    const ModelObjectData l_object = FetchModelObjectData();
    const ModelMeshData   l_mesh   = FetchModelMeshData  ();

    // 1つのthreadが1つのMeshletを担当する
    const uint l_meshletIndex   = a_dispatchThreadID.x;
          bool l_shouldDispatch = false;

    // 最後のASGroupには実際のMeshlet数を超えるThreadが含まれる
    // 範囲内のThreadだけがMeshletBoundsを参照する
    if (l_meshletIndex < l_mesh.meshletCount)
    {
        l_shouldDispatch = ShouldDispatchModelCascadeShadowMeshlet(l_object, l_mesh, l_meshletIndex);
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