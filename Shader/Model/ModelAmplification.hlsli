#ifndef MODEL_AMPLIFICATION_HLSLI
#define MODEL_AMPLIFICATION_HLSLI
#include "ModelMeshletCulling.hlsli"

groupshared ModelAmplificationPayload g_modelAmplificationPayload;
groupshared uint                      g_modelVisibleMeshletCount;

// 通常の描画では、MeshletのBoundingSphereをそのままの大きさで判定する
static const float k_modelNoMeshletRadiusExpansion = 0.0F;

static const bool k_modelMeshletBackfaceCullingEnabled  = true;
static const bool k_modelMeshletBackfaceCullingDisabled = false;

// Model共通のAmplificationShaderの処理
// 1つのThreadが1つのMeshletのカリングを担当し、見えているMeshletの数だけMeshShaderGroupを起動する
// StaticModel、SkeletalAnimationModel、GPUスキニング、輪郭線のAmplificationShaderは、
// エントリー関数からこの関数を呼ぶだけにして、カリングの流れを1か所にまとめている
// a_worldRadiusExpansion     : Frustumの判定でMeshletのBoundingSphereの半径に足す距離(ワールド座標)
//                              通常の描画は0、輪郭線は頂点を押し出す太さを渡す
// a_isBackfaceCullingEnabled : falseなら裏向きのMeshletも消さない
//                              輪郭線は「裏面を外側へ押し出して描く」ため、裏向きのMeshletこそ必要になる
void DispatchModelAmplification(const uint3 a_dispatchThreadID,
                                const uint  a_groupThreadIndex,
                                const float a_worldRadiusExpansion,
                                const bool  a_isBackfaceCullingEnabled)
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

    bool l_shouldDispatch    = false;
    uint l_cullingDebugState = k_modelCullingDebugStateVisible;

    // 最後のAmplificationShaderGroupには、
    // 実際のMeshlet数を超えた余分なThreadが含まれる可能性がある
    // 範囲外のThreadはMeshletBoundsBufferへアクセスさせない
    if (l_lodMeshletIndex < g_meshletCount)
    {
        // GPUスキニングするMeshは、Meshletの範囲(元の姿勢で計算した物)が動いた形と合わないため、
        // Meshlet単位のカリングをせずに全て描画する(Mesh単位のカリングは動いた形を囲む球で済ませている)
        l_shouldDispatch = g_isGPUSkinning != k_modelFalseValue ? true : ShouldDispatchModelMeshlet(l_meshletIndex,
                                                                                                     a_worldRadiusExpansion,
                                                                                                     a_isBackfaceCullingEnabled,
                                                                                                     l_cullingDebugState);
    }

    if (l_shouldDispatch)
    {
        uint l_payloadMeshletIndex = k_modelAmplificationInitialVisibleMeshletCount;

        // 可視Meshletを書き込む配列位置を一つ確保する
        // InterlockedAddの第三引数には加算前の値が入るため、
        // 可視Meshletは配列のZero番目から連続して格納される
        InterlockedAdd(g_modelVisibleMeshletCount, k_modelAmplificationVisibleMeshletCountIncrement, l_payloadMeshletIndex);

        // Meshletの番号と一緒に、カリングの可視化用の判定結果を上位ビットへ入れて渡す(通常は「見えている」)
        g_modelAmplificationPayload.meshletIndexList[l_payloadMeshletIndex] = EncodeModelCullingDebugMeshletIndex(l_meshletIndex, l_cullingDebugState);
    }

    // DispatchMesh自体にGroupMemoryBarrierWithGroupSync相当の同期が含まれる
    // 可視判定を通過したMeshlet数だけ、子となるMeshShaderGroupを起動する
    // 可視Meshletが存在しなければ、GroupCountXは0となりMeshShaderは起動されない
    DispatchMesh(g_modelVisibleMeshletCount,
                 k_modelAmplificationDispatchMeshGroupCountY,
                 k_modelAmplificationDispatchMeshGroupCountZ,
                 g_modelAmplificationPayload);
}

#endif // MODEL_AMPLIFICATION_HLSLI