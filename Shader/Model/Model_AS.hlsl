#include "ModelMeshletCulling.hlsli"

groupshared ModelAmplificationPayload g_modelAmplificationPayload;
groupshared uint                      g_modelVisibleMeshletCount;

// Static / Skeletal 共通のAmplificationShader
// どちらも「メッシュのテーブルが指すバッファ」を読むだけなので、同じシェーダーで済む
// (Skeletalは、メッシュのテーブルがスキニング後の頂点と、計算し直したMeshletの境界を指している)
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

    // この描画のオブジェクト(行列)とメッシュ(バッファの番号・Meshletの数)を、テーブルから読む
    // 32個のThreadが同じ要素を読むが、GPUのキャッシュに乗るため、読み込みは実質1回分で済む
    const ModelObjectData l_object = FetchModelObjectData();
    const ModelMeshData   l_mesh   = FetchModelMeshData  ();

    // SV_DispatchThreadID.xは、全てのAmplificationShaderGroupを通した連続ThreadIndex
    // 1つのThreadが1つのMeshletを担当するため、
    // この値をそのままMeshletIndexとして使用できる
    const uint l_meshletIndex   = a_dispatchThreadID.x;
          bool l_shouldDispatch = false;

    // 最後のAmplificationShaderGroupには、
    // 実際のMeshlet数を超えた余分なThreadが含まれる可能性がある
    // 範囲外のThreadはMeshletBoundsBufferへアクセスさせない
    if (l_meshletIndex < l_mesh.meshletCount)
    {
        l_shouldDispatch = ShouldDispatchModelMeshlet(l_object, l_mesh, l_meshletIndex);
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

    // DispatchMesh自体にGroupMemoryBarrierWithGroupSync相当の同期が含まれる
    // 可視判定を通過したMeshlet数だけ、子となるMeshShaderGroupを起動する
    // 可視Meshletが存在しなければ、GroupCountXは0となりMeshShaderは起動されない
    DispatchMesh(g_modelVisibleMeshletCount,
                 k_modelAmplificationDispatchMeshGroupCountY,
                 k_modelAmplificationDispatchMeshGroupCountZ,
                 g_modelAmplificationPayload);
}