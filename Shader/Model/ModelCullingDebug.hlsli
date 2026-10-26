#ifndef MODEL_CULLING_DEBUG_HLSLI
#define MODEL_CULLING_DEBUG_HLSLI

// カリング結果の可視化(エディターのデバッグカメラ用)で使う値
//
// 通常の描画では、視錐台の外や裏向きと判定したMeshやMeshletは描画されない
// デバッグカメラで「メインカメラがどこを消しているか」を見るときは、消すはずの物も描画し、
// 判定結果ごとの色を混ぜる(見えている物はそのままの色)
//
// 判定結果は2段階で決まる
// 1. Mesh単位      : GPUドリブン描画のカリング(ModelGPUDrivenCulling_CS)が、Meshを囲む球で判定して定数バッファ(g_cullingDebugState)へ書く
// 2. Meshlet単位   : AmplificationShaderがMeshletごとに判定し、Payloadの番号の上位ビットへ入れてMeshShaderへ渡す
// ※ C++側(Definition/Constant/Graphics/Buffer/Constant/CBModelPerObjectStructConstant.h)の初期値0は「見えている」と同じ値にする

static const uint k_modelCullingDebugStateVisible               = 0U;
static const uint k_modelCullingDebugStateMeshFrustumCulled     = 1U;
static const uint k_modelCullingDebugStateMeshOccluded          = 2U;
static const uint k_modelCullingDebugStateMeshletFrustumCulled  = 3U;
static const uint k_modelCullingDebugStateMeshletBackfaceCulled = 4U;

// Payloadの番号の上位3bitへ判定結果を入れる(Meshletの番号は下位29bitに収まる前提。最大約5億個)
static const uint k_modelCullingDebugStateShiftBit = 29U;
static const uint k_modelCullingDebugMeshletIndexMask = 0x1FFFFFFFU;
static const uint k_modelCullingDebugStateMask        = 0x7U;

// 判定結果ごとに混ぜる色と、混ぜる割合
static const float3 k_modelCullingDebugMeshFrustumCulledColor     = float3(1.0F, 0.15F, 0.15F);
static const float3 k_modelCullingDebugMeshOccludedColor          = float3(1.0F, 0.2F,  1.0F);
static const float3 k_modelCullingDebugMeshletFrustumCulledColor  = float3(1.0F, 0.6F,  0.1F);
static const float3 k_modelCullingDebugMeshletBackfaceCulledColor = float3(0.2F, 0.5F,  1.0F);

static const float k_modelCullingDebugTintWeight = 0.7F;

// Meshletの番号と判定結果を1つのuintへまとめる(AmplificationShaderがPayloadへ書くときに使う)
uint EncodeModelCullingDebugMeshletIndex(const uint a_meshletIndex, const uint a_cullingDebugState)
{
    return (a_meshletIndex & k_modelCullingDebugMeshletIndexMask) | (a_cullingDebugState << k_modelCullingDebugStateShiftBit);
}

// まとめたuintからMeshletの番号を取り出す(MeshShaderで使う)
uint DecodeModelCullingDebugMeshletIndex(const uint a_packedMeshletIndex)
{
    return a_packedMeshletIndex & k_modelCullingDebugMeshletIndexMask;
}

// まとめたuintからMeshlet単位の判定結果を取り出す(MeshShaderで使う)
uint DecodeModelCullingDebugState(const uint a_packedMeshletIndex)
{
    return (a_packedMeshletIndex >> k_modelCullingDebugStateShiftBit) & k_modelCullingDebugStateMask;
}

// Mesh単位の判定結果とMeshlet単位の判定結果から、Pixelへ渡す判定結果を決める
// Mesh単位で消されていればそちらを優先する(Meshごと消えているのでMeshlet単位の判定に意味が無いため)
uint SelectModelCullingDebugState(const uint a_meshCullingDebugState, const uint a_meshletCullingDebugState)
{
    if (a_meshCullingDebugState != k_modelCullingDebugStateVisible) { return a_meshCullingDebugState; }

    return a_meshletCullingDebugState;
}

// 判定結果に合わせて色を混ぜる(見えている物はそのまま返す)
float4 ApplyModelCullingDebugTint(const float4 a_color, const uint a_cullingDebugState)
{
    float3 l_tintColor = a_color.rgb;

    switch (a_cullingDebugState)
    {
        case k_modelCullingDebugStateMeshFrustumCulled:     l_tintColor = k_modelCullingDebugMeshFrustumCulledColor;     break;
        case k_modelCullingDebugStateMeshOccluded:          l_tintColor = k_modelCullingDebugMeshOccludedColor;          break;
        case k_modelCullingDebugStateMeshletFrustumCulled:  l_tintColor = k_modelCullingDebugMeshletFrustumCulledColor;  break;
        case k_modelCullingDebugStateMeshletBackfaceCulled: l_tintColor = k_modelCullingDebugMeshletBackfaceCulledColor; break;
        default:                                            return a_color;
    }

    return float4(lerp(a_color.rgb, l_tintColor, k_modelCullingDebugTintWeight), a_color.a);
}

#endif // MODEL_CULLING_DEBUG_HLSLI
