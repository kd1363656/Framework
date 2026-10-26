#include "ModelGPUDriven.hlsli"

// GPUドリブン描画のカリング用ComputeShader
//
// 1つのThreadが「インスタンス1体 × Mesh1つ」の組み合わせを担当し、
// 1. 視錐台(カメラ又はライトから見える範囲)に入っているか判定する
// 2. 手前の物に完全に隠されていないか、Hi-Z(階層深度バッファ)で判定する(オクルージョンカリング)
// 3. 見えていれば、カメラからの距離に応じてLODを選ぶ
// 4. 描画1回分の定数バッファ(DrawRecord)と、ExecuteIndirect用の描画コマンドを書き込む
//
// オクルージョンカリングは2回に分けて行う
// 1回目(Early) : 前のフレームのHi-Zで判定する。隠れていた物は「候補」として記録だけして描画しない
// 2回目(Late)  : 1回目で描いた物から今のフレームのHi-Zを作り、候補だけを判定し直して描画する
// こうすることで、前のフレームでは隠れていたが今は見えている物も、描き漏らさずに済む

// 視錐台を作る平面の最大数(C++側のConstant::k_maxGPUDrivenFrustumPlaneCountと必ず同じ値にする)
static const uint k_maxFrustumPlaneCount = 6U;

cbuffer CBModelGPUDrivenCullingPass : register(b0)
{
    // 視錐台を作る平面(xyz : 平面の法線、w : 原点からの距離)
    float4 g_frustumPlaneList[k_maxFrustumPlaneCount];

    float3 g_cameraWorldPosition;
    float  g_lodProjectionScale;

    float g_lodMaxScreenSpaceErrorPixel;
    float g_lodMinDistance;
    uint  g_frustumPlaneCount;
    uint  g_maxDrawCount;

    uint g_instanceOffset;
    uint g_instanceCount;
    uint g_meshInfoOffset;
    uint g_meshCount;

    uint g_instanceBufferSRVDescriptorIndex;
    uint g_meshInfoBufferSRVDescriptorIndex;
    uint g_lodBufferSRVDescriptorIndex;
    uint g_drawRecordBufferUAVDescriptorIndex;

    uint  g_indirectCommandBufferUAVDescriptorIndex;
    uint  g_counterBufferUAVDescriptorIndex;
    uint2 g_drawRecordBufferAddress;

    // オクルージョンカリングで使うビュープロジェクション行列(1回目は前のフレーム、2回目は今のフレーム)
    row_major float4x4 g_occlusionViewProjectionMatrix;

    uint g_cullingMode;
    uint g_isOcclusionEnabled;
    uint g_hierarchicalDepthTextureSRVDescriptorIndex;
    uint g_hierarchicalDepthMIPCount;

    float2 g_hierarchicalDepthSize;
    uint   g_candidateBufferUAVDescriptorIndex;
    uint   g_candidateCounterBufferUAVDescriptorIndex;

    // g_morphOverrideWeightBufferSRVDescriptorIndex : インスタンスごとに上書きするモーフのウェイトのBuffer
    uint  g_maxCandidateCount;
    uint  g_isOrthographicLOD;
    float g_minProjectedSizeTexel;
    uint  g_morphOverrideWeightBufferSRVDescriptorIndex;

    // g_isCullingDebugVisualize : 1なら視錐台の外、又は隠れていると判定した組み合わせも、判定結果を付けて描画コマンドを書く(カリングの可視化)
    uint  g_isCullingDebugVisualize;
    uint3 g_cullingPassPadding;
};

// オクルージョンカリングの候補(1回目で隠れていると判定された組み合わせ)
struct ModelGPUDrivenOcclusionCandidate
{
    uint instanceIndex;
    uint meshInfoIndex;
};

// C++側のConstant::k_modelGPUDrivenCullingThreadCountと必ず同じ値にする
static const uint k_modelGPUDrivenCullingThreadCountX = 64U;
static const uint k_modelGPUDrivenCullingThreadCountY = 1U;
static const uint k_modelGPUDrivenCullingThreadCountZ = 1U;

// カリングの種類(C++側のEnum::ModelGPUDrivenCullingModeと同じ値)
static const uint k_cullingModeNormal = 0U;
static const uint k_cullingModeEarly  = 1U;
static const uint k_cullingModeLate   = 2U;

// CounterBufferの要素番号(C++側のConstant::k_modelGPUDrivenCounterElementCountと数を合わせる)
static const uint k_drawCountCounterIndex      = 0U;
static const uint k_candidateCountCounterIndex = 1U;

// 1つのAmplificationShaderGroupが担当するMeshlet数(C++側と同じ)
static const uint k_meshletCountPerAmplificationShaderGroup = 32U;

static const uint k_dispatchMeshGroupCountY = 1U;
static const uint k_dispatchMeshGroupCountZ = 1U;

static const uint k_drawRecordByteSize = 256U;

static const uint k_trueValue = 1U;

// C++側のDescriptorHeap::k_invalidDescriptorIndexと同じ値(無効なDescriptorの番号)
static const uint k_invalidDescriptorIndex = 0xFFFFFFFFU;

static const uint k_firstLODIndex = 0U;

// C++側のConstant::k_invalidGPUDrivenMorphWeightOffsetと同じ値(インスタンスごとのウェイトを使わない)
static const uint k_invalidMorphWeightOffset = 0xFFFFFFFFU;

static const float k_initialMorphWeightScale = 0.0F;

static const uint k_boundsCornerCount = 8U;

static const float k_positionElementW = 1.0F;

// クリップ座標のwがこれ以下の点はカメラの後ろにある
static const float k_minClipW = 0.0001F;

// 画面座標(-1～1)からUV(0～1)への変換
static const float2 k_ndcToUVScale  = float2(0.5F, -0.5F);
static const float2 k_ndcToUVOffset = float2(0.5F,  0.5F);

// 深度比較の誤差を吸収するための値
static const float k_occlusionDepthBias = 0.00001F;

// 平行投影(影)のLOD選択で使う距離(距離で割らないようにするため1にする)
static const float k_orthographicLODDistance = 1.0F;

static const float k_diameterScale = 2.0F;

// 「数」から「最後の番号」へ直すときに引く値(例 : ミップ数 - 1 = 最後のミップ番号)
static const uint k_countToLastIndexOffset = 1U;

// 構造体を全て0で初期化するときに使う値
static const uint k_zeroInitializeValue = 0U;

// InterlockedAddでカウンターを1つ進めるときに足す値
static const uint k_counterIncrementValue = 1U;

static const uint k_initialCounterValue = 0U;
static const uint k_initialIndex        = 0U;
static const uint k_initialOffset       = 0U;
static const uint k_emptyFrameCount     = 0U;

// GPUアドレスの下位32bitが繰り上がったときに上位32bitへ足す値
static const uint k_addressCarryValue   = 1U;
static const uint k_addressNoCarryValue = 0U;

// UVの範囲(0～1)と、深度の範囲(0が一番手前、1が一番奥)
static const float k_minUV         = 0.0F;
static const float k_maxUV         = 1.0F;
static const float k_farthestDepth = 1.0F;

// 箱の角の番号のビット(1bit目がx、2bit目がy、3bit目がz)で、その軸を+側にするか-側にするかを決める
static const uint k_boundsCornerBitX = 1U;
static const uint k_boundsCornerBitY = 2U;
static const uint k_boundsCornerBitZ = 4U;

static const float k_positiveCornerSign =  1.0F;
static const float k_negativeCornerSign = -1.0F;

// Hi-Z上で物体が覆う大きさ(テクセル)の最小値(1テクセルより小さくてもミップ0を使う)
static const float k_minCoveredTexelSize = 1.0F;

static const float k_firstMIPLevel = 0.0F;

// ミップを縮小していったときの最小の大きさ(1×1テクセル)
static const uint2 k_minMIPSize = uint2(1U, 1U);


// 球(中心と半径)が視錐台の内側に少しでも入っているか判定する
// 平面の法線は視錐台の内側を向いているため、
// 中心から平面までの距離が -半径 より小さければ、球は完全に外側にある
bool IsVisibleBoundingSphere(const float3 a_worldCenter, const float a_worldRadius)
{
    for (uint l_planeIndex = 0U; l_planeIndex < g_frustumPlaneCount; ++l_planeIndex)
    {
        const float4 l_plane    = g_frustumPlaneList[l_planeIndex];
        const float  l_distance = dot(l_plane.xyz, a_worldCenter) + l_plane.w;

        if (l_distance < -a_worldRadius) { return false; }
    }

    return true;
}

// 球がHi-Zで表される「手前の物」に完全に隠れているか判定する
bool IsOccludedBoundingSphere(const float3 a_worldCenter, const float a_worldRadius)
{
    if (g_isOcclusionEnabled != k_trueValue) { return false; }

    // 1. 球を囲む箱の8つの角を画面へ投影し、画面上で覆う範囲(UV)と一番手前の深度を求める
    float2 l_uvMIN        = float2(k_maxUV, k_maxUV);
    float2 l_uvMAX        = float2(k_minUV, k_minUV);
    float  l_nearestDepth = k_farthestDepth;

    for (uint l_cornerIndex = 0U; l_cornerIndex < k_boundsCornerCount; ++l_cornerIndex)
    {
        // 角の番号のビットで、x、y、zそれぞれを-側にするか+側にするかを決める
        const float3 l_cornerSign = float3((l_cornerIndex & k_boundsCornerBitX) ? k_positiveCornerSign : k_negativeCornerSign,
                                           (l_cornerIndex & k_boundsCornerBitY) ? k_positiveCornerSign : k_negativeCornerSign,
                                           (l_cornerIndex & k_boundsCornerBitZ) ? k_positiveCornerSign : k_negativeCornerSign);

        const float3 l_corner = a_worldCenter + l_cornerSign * a_worldRadius;
        const float4 l_clip   = mul(float4(l_corner, k_positionElementW), g_occlusionViewProjectionMatrix);

        // カメラの後ろにかかる物は投影できないため、隠れていないとみなす
        if (l_clip.w <= k_minClipW) { return false; }

        const float3 l_ndc = l_clip.xyz / l_clip.w;
        const float2 l_uv  = l_ndc.xy * k_ndcToUVScale + k_ndcToUVOffset;

        l_uvMIN        = min(l_uvMIN, l_uv);
        l_uvMAX        = max(l_uvMAX, l_uv);
        l_nearestDepth = min(l_nearestDepth, l_ndc.z);
    }

    l_uvMIN = saturate(l_uvMIN);
    l_uvMAX = saturate(l_uvMAX);

    // 2. 覆う範囲がHi-Zの2×2ピクセル以内に収まるミップを選ぶ
    //    (大きい範囲ほど縮小されたミップを使うことで、読むピクセル数を常に4つにできる)
    const float2 l_sizeInTexel    = (l_uvMAX - l_uvMIN) * g_hierarchicalDepthSize;
    const float  l_maxSizeInTexel = max(max(l_sizeInTexel.x, l_sizeInTexel.y), k_minCoveredTexelSize);
    const float  l_lastMIPLevel   = (float)(g_hierarchicalDepthMIPCount - k_countToLastIndexOffset);
    const float  l_mipLevel       = clamp(ceil(log2(l_maxSizeInTexel)), k_firstMIPLevel, l_lastMIPLevel);

    Texture2D<float> l_hierarchicalDepthTexture = ResourceDescriptorHeap[g_hierarchicalDepthTextureSRVDescriptorIndex];

    const uint  l_mip       = (uint)l_mipLevel;
    const uint2 l_mipSize   = max(uint2(g_hierarchicalDepthSize) >> l_mip, k_minMIPSize);
    const uint2 l_lastTexel = l_mipSize - k_countToLastIndexOffset;
    const uint2 l_texelMIN  = min(uint2(l_uvMIN * float2(l_mipSize)), l_lastTexel);
    const uint2 l_texelMAX  = min(uint2(l_uvMAX * float2(l_mipSize)), l_lastTexel);

    // 3. 範囲の四隅のピクセルから、一番奥の深度を求める
    const float l_depth0 = l_hierarchicalDepthTexture.Load(int3(l_texelMIN.x, l_texelMIN.y, l_mip));
    const float l_depth1 = l_hierarchicalDepthTexture.Load(int3(l_texelMAX.x, l_texelMIN.y, l_mip));
    const float l_depth2 = l_hierarchicalDepthTexture.Load(int3(l_texelMIN.x, l_texelMAX.y, l_mip));
    const float l_depth3 = l_hierarchicalDepthTexture.Load(int3(l_texelMAX.x, l_texelMAX.y, l_mip));

    const float l_farthestDepth = max(max(l_depth0, l_depth1), max(l_depth2, l_depth3));

    // 4. 物体の一番手前でさえ、範囲内の一番奥より奥にあるなら、完全に隠れている
    return l_nearestDepth > l_farthestDepth + k_occlusionDepthBias;
}

// このMeshで効いているモーフのウェイトの最大値を求める(このMeshが使うチャンネルだけを調べる)
// 0なら、どのターゲットも効いていないので、LODを選ぶときにモーフの崩れを足さずに済む
float CalculateMorphWeightScale(const ModelGPUDrivenMeshInfo a_meshInfo, const uint a_morphWeightBufferSRVDescriptorIndex, const uint a_morphWeightOffset)
{
    StructuredBuffer<uint>  l_morphChannelListBuffer = ResourceDescriptorHeap[a_meshInfo.morphChannelListBufferSRVDescriptorIndex];
    StructuredBuffer<float> l_morphWeightBuffer      = ResourceDescriptorHeap[a_morphWeightBufferSRVDescriptorIndex];

    float l_morphWeightScale = k_initialMorphWeightScale;

    for (uint l_channelListIndex = 0U; l_channelListIndex < a_meshInfo.morphChannelListCount; ++l_channelListIndex)
    {
        const uint l_channelIndex = l_morphChannelListBuffer[l_channelListIndex];

        // ウェイトは負の値(逆向きに動かす)も使えるため、絶対値で比べる
        l_morphWeightScale = max(l_morphWeightScale, abs(l_morphWeightBuffer[a_morphWeightOffset + l_channelIndex]));
    }

    return l_morphWeightScale;
}

// LOD1段階分の、効かせているターゲットの分だけのモーフの崩れを求める
// 崩れ = Σ(ターゲットのウェイトの絶対値 × そのターゲットをウェイト1で効かせたときの崩れ)
// (口だけを効かせているなら口のターゲットの崩れだけが足されるので、体まで細かいLODにならない)
// a_localLODIndex : Mesh内でのLOD番号(0がLOD0)
float CalculateLODMorphError(const ModelGPUDrivenMeshInfo a_meshInfo, const uint a_morphWeightBufferSRVDescriptorIndex, const uint a_morphWeightOffset, const uint a_localLODIndex)
{
    StructuredBuffer<uint>  l_morphChannelListBuffer    = ResourceDescriptorHeap[a_meshInfo.morphChannelListBufferSRVDescriptorIndex];
    StructuredBuffer<float> l_morphWeightBuffer         = ResourceDescriptorHeap[a_morphWeightBufferSRVDescriptorIndex];
    StructuredBuffer<float> l_morphLODTargetErrorBuffer = ResourceDescriptorHeap[a_meshInfo.morphLODTargetErrorBufferSRVDescriptorIndex];

    const uint l_targetErrorOffset = a_localLODIndex * a_meshInfo.morphChannelListCount;

    float l_morphError = k_initialMorphWeightScale;

    for (uint l_targetIndex = 0U; l_targetIndex < a_meshInfo.morphChannelListCount; ++l_targetIndex)
    {
        const float l_weight = abs(l_morphWeightBuffer[a_morphWeightOffset + l_morphChannelListBuffer[l_targetIndex]]);

        // 効いていないターゲットは崩れも0なので、崩れを読まない
        if (l_weight <= k_initialMorphWeightScale) { continue; }

        l_morphError += l_weight * l_morphLODTargetErrorBuffer[l_targetErrorOffset + l_targetIndex];
    }

    return l_morphError;
}

// カメラから、Meshを囲む球の表面までの距離(LODの選び方と、覆われている頂点を奥へ押し出す距離の計算で同じ距離を使う)
// 影(平行投影)では距離によって大きさが変わらないため、距離を1として扱う
// (g_lodProjectionScaleには「1メートルがシャドウマップの何テクセルか」が入っている)
float CalculateLODDistance(const float3 a_worldCenter, const float a_worldRadius)
{
    if (g_isOrthographicLOD == k_trueValue) { return k_orthographicLODDistance; }

    return max(distance(g_cameraWorldPosition, a_worldCenter) - a_worldRadius, g_lodMinDistance);
}

// 画面上の誤差が許容値以下になる、最も三角形の少ないLODを選ぶ
// (C++側のModelLODSelectorと同じ計算に、効かせているターゲットの分のモーフの崩れを足した物)
// a_morphWeightBufferSRVDescriptorIndex : このインスタンスのウェイトのBuffer(無効値ならモーフの崩れを足さない)
// a_morphWeightOffset                   : このインスタンスのウェイトがBufferの何番目から始まるか
ModelGPUDrivenMeshLOD SelectMeshLOD(const ModelGPUDrivenMeshInfo a_meshInfo,
                                    const float3                 a_worldCenter,
                                    const float                  a_worldRadius,
                                    const float                  a_worldMaxScale,
                                    const uint                   a_morphWeightBufferSRVDescriptorIndex,
                                    const uint                   a_morphWeightOffset)
{
    StructuredBuffer<ModelGPUDrivenMeshLOD> l_lodBuffer = ResourceDescriptorHeap[g_lodBufferSRVDescriptorIndex];

    const float l_distance = CalculateLODDistance(a_worldCenter, a_worldRadius);

    // 三角形が一番少ないLODから順に調べる
    for (uint l_lodIndex = a_meshInfo.lodCount; l_lodIndex > k_firstLODIndex; --l_lodIndex)
    {
        const uint                  l_localLODIndex = l_lodIndex - k_countToLastIndexOffset;
        const ModelGPUDrivenMeshLOD l_lod           = l_lodBuffer[a_meshInfo.lodOffset + l_localLODIndex];

        // LODの崩れ = 形の崩れ + 効かせているターゲットの分のモーフの崩れ
        float l_lodError = l_lod.simplifyError;

        if (a_morphWeightBufferSRVDescriptorIndex != k_invalidDescriptorIndex)
        {
            l_lodError += CalculateLODMorphError(a_meshInfo, a_morphWeightBufferSRVDescriptorIndex, a_morphWeightOffset, l_localLODIndex);
        }

        const float l_screenSpaceErrorPixel = l_lodError * a_worldMaxScale * g_lodProjectionScale / l_distance;

        if (l_screenSpaceErrorPixel <= g_lodMaxScreenSpaceErrorPixel) { return l_lod; }
    }

    // どのLODも許容値を超える場合は元の形(LOD0)を使う
    return l_lodBuffer[a_meshInfo.lodOffset];
}

// 描画1回分の定数バッファと描画コマンドを書き込む
// a_cullingDebugState : カリングの可視化用の判定結果(通常の描画では常に「見えている」)
void WriteDrawCommand(const ModelGPUDrivenInstance a_instance,
                      const ModelGPUDrivenMeshInfo a_meshInfo,
                      const float3                 a_worldCenter,
                      const float                  a_worldRadius,
                      const uint                   a_cullingDebugState)
{
    // モーフを持つMeshなら、このインスタンスのウェイトがどこにあるかを決める
    // ・インスタンスごとのウェイトがあれば、上書き用ウェイトBufferの指定された位置
    // ・無ければ、ベイクしたウェイトの「このコマ」の位置
    uint  l_morphWeightBufferSRVDescriptorIndex = k_invalidDescriptorIndex;
    uint  l_morphWeightOffset                   = k_initialOffset;
    float l_morphWeightScale                    = k_initialMorphWeightScale;

    if (a_meshInfo.morphDeltaBufferSRVDescriptorIndex != k_invalidDescriptorIndex)
    {
        if (a_instance.morphWeightOffset != k_invalidMorphWeightOffset)
        {
            l_morphWeightBufferSRVDescriptorIndex = g_morphOverrideWeightBufferSRVDescriptorIndex;
            l_morphWeightOffset                   = a_instance.morphWeightOffset;
        }
        else if (a_meshInfo.morphWeightBufferSRVDescriptorIndex != k_invalidDescriptorIndex &&
                 a_meshInfo.skinningFrameCount                  >  k_emptyFrameCount)
        {
            l_morphWeightBufferSRVDescriptorIndex = a_meshInfo.morphWeightBufferSRVDescriptorIndex;
            l_morphWeightOffset                   = (a_instance.animationFrameIndex % a_meshInfo.skinningFrameCount) * a_meshInfo.morphWeightStride;
        }

        if (l_morphWeightBufferSRVDescriptorIndex != k_invalidDescriptorIndex)
        {
            l_morphWeightScale = CalculateMorphWeightScale(a_meshInfo, l_morphWeightBufferSRVDescriptorIndex, l_morphWeightOffset);
        }

        // どのターゲットも効いていなければ(ウェイトが全て0)、形は元のままなのでモーフは無いものとして扱う
        // (LODにモーフの崩れを足さず、メッシュシェーダーも移動量を1つも読まずに済む)
        if (l_morphWeightScale <= k_initialMorphWeightScale) { l_morphWeightBufferSRVDescriptorIndex = k_invalidDescriptorIndex; }
    }

    // LODを選ぶ(LODが無いMeshは全Meshletを描画する)
    uint l_meshletOffset = k_initialOffset;
    uint l_meshletCount  = a_meshInfo.allMeshletCount;

    if (a_meshInfo.lodCount > k_firstLODIndex)
    {
        const ModelGPUDrivenMeshLOD l_lod = SelectMeshLOD(a_meshInfo,
                                                          a_worldCenter,
                                                          a_worldRadius,
                                                          a_instance.worldMaxScale,
                                                          l_morphWeightBufferSRVDescriptorIndex,
                                                          l_morphWeightOffset);

        l_meshletOffset = l_lod.meshletOffset;
        l_meshletCount  = l_lod.meshletCount;
    }

    // 描画コマンドの書き込み先を1つ確保する
    // InterlockedAddは複数Threadが同時に足しても正しく数えられる加算で、
    // 第三引数に「加算前の値」が入るため、それを自分専用の書き込み位置として使う
    RWStructuredBuffer<uint> l_counterBuffer = ResourceDescriptorHeap[g_counterBufferUAVDescriptorIndex];

    uint l_drawIndex = k_initialCounterValue;

    InterlockedAdd(l_counterBuffer[k_drawCountCounterIndex], k_counterIncrementValue, l_drawIndex);

    // 用意したBufferの数を超えた分は描画しない
    // (ExecuteIndirectもC++側で最大数を指定しているため、数が超えても範囲外は読まれない)
    if (l_drawIndex >= g_maxDrawCount) { return; }

    // 描画1回分の定数バッファを書き込む
    RWStructuredBuffer<ModelGPUDrivenDrawRecord> l_drawRecordBuffer = ResourceDescriptorHeap[g_drawRecordBufferUAVDescriptorIndex];

    ModelGPUDrivenDrawRecord l_drawRecord = (ModelGPUDrivenDrawRecord)k_zeroInitializeValue;

    l_drawRecord.worldMatrix                 = a_instance.worldMatrix;
    l_drawRecord.worldInverseTransposeMatrix = a_instance.worldInverseTransposeMatrix;

    l_drawRecord.materialBufferSRVDescriptorIndex          = a_meshInfo.materialBufferSRVDescriptorIndex;
    l_drawRecord.vertexBufferSRVDescriptorIndex            = a_meshInfo.vertexBufferSRVDescriptorIndex;
    l_drawRecord.meshletBufferSRVDescriptorIndex           = a_meshInfo.meshletBufferSRVDescriptorIndex;
    l_drawRecord.uniqueVertexIndexBufferSRVDescriptorIndex = a_meshInfo.uniqueVertexIndexBufferSRVDescriptorIndex;
    l_drawRecord.primitiveIndexBufferSRVDescriptorIndex    = a_meshInfo.primitiveIndexBufferSRVDescriptorIndex;
    l_drawRecord.meshletBoundsBufferSRVDescriptorIndex     = a_meshInfo.meshletBoundsBufferSRVDescriptorIndex;

    l_drawRecord.worldMaxScale        = a_instance.worldMaxScale;
    l_drawRecord.worldOrientationSign = a_instance.worldOrientationSign;
    l_drawRecord.meshletCount         = l_meshletCount;
    l_drawRecord.meshletOffset        = l_meshletOffset;
    l_drawRecord.cullingDebugState    = a_cullingDebugState;

    // 他のMeshに覆われている頂点を、覆われている層1つ分だけカメラから見て奥へ押し出す距離(ワールド座標。0以上)
    // どのMeshも「画面上の誤差が許容値以下」のLODで描くため、表面のずれはこの位置で
    // 許容値(ピクセル) × 距離 ÷ g_lodProjectionScale の長さ以下になる。その長さに、実際のずれの倍率を掛けて押し出す
    // (覆う側の服が沈んでも、覆われている肌はさらにその奥になり、透けて見えなくなる)
    // 影の描画では、隠れている面の前後関係は影の形に関係しないので押し出さない
    l_drawRecord.lodCoveredLayerCountBufferSRVDescriptorIndex = a_meshInfo.lodCoveredLayerCountBufferSRVDescriptorIndex;

    if (g_isOrthographicLOD != k_trueValue)
    {
        const float l_lodDistance = CalculateLODDistance(a_worldCenter, a_worldRadius);

        l_drawRecord.lodCoveredLayerDepthOffset = a_meshInfo.lodSurfaceDeviationScale * g_lodMaxScreenSpaceErrorPixel * l_lodDistance / g_lodProjectionScale;
    }

    // スキニングするモデルなら、このインスタンスのアニメーションのコマに対応するスキニング行列の位置を求める
    if (a_meshInfo.skinningMatrixBufferSRVDescriptorIndex != k_invalidDescriptorIndex &&
        a_meshInfo.skinningFrameCount                     >  k_emptyFrameCount)
    {
        const uint l_frameIndex = a_instance.animationFrameIndex % a_meshInfo.skinningFrameCount;

        l_drawRecord.skinningMatrixBufferSRVDescriptorIndex = a_meshInfo.skinningMatrixBufferSRVDescriptorIndex;
        l_drawRecord.skinningMatrixOffset                   = l_frameIndex * a_meshInfo.skinningPaletteStride + a_meshInfo.skinningPaletteBase;
        l_drawRecord.isGPUSkinning                          = k_trueValue;
    }

    // モーフを持ち、0でないウェイトがある場合だけ、メッシュシェーダーがスキニングの前にモーフを足す
    // (ウェイトが全て0でも足す処理を行うと、メッシュシェーダーが頂点ごとに全ての移動量を読んでから読み飛ばすことになり、
    //  500体の負荷テストでGPU時間が約4倍になった)
    if (l_morphWeightBufferSRVDescriptorIndex != k_invalidDescriptorIndex)
    {
        l_drawRecord.morphVertexRangeBufferSRVDescriptorIndex = a_meshInfo.morphVertexRangeBufferSRVDescriptorIndex;
        l_drawRecord.morphDeltaBufferSRVDescriptorIndex       = a_meshInfo.morphDeltaBufferSRVDescriptorIndex;
        l_drawRecord.morphWeightBufferSRVDescriptorIndex      = l_morphWeightBufferSRVDescriptorIndex;
        l_drawRecord.morphWeightOffset                        = l_morphWeightOffset;
    }
    else
    {
        l_drawRecord.morphVertexRangeBufferSRVDescriptorIndex = k_invalidDescriptorIndex;
        l_drawRecord.morphDeltaBufferSRVDescriptorIndex       = k_invalidDescriptorIndex;
        l_drawRecord.morphWeightBufferSRVDescriptorIndex      = k_invalidDescriptorIndex;
    }

    l_drawRecordBuffer[l_drawIndex] = l_drawRecord;

    // 描画コマンドを書き込む
    // 定数バッファのアドレス = DrawRecordBufferの先頭アドレス + 番号 × 256byte
    // GPUのアドレスは64bitなので、下位32bitと上位32bitに分けて扱い、繰り上がりも計算する
    const uint l_addressOffset = l_drawIndex * k_drawRecordByteSize;
    const uint l_addressLow    = g_drawRecordBufferAddress.x + l_addressOffset;
    const uint l_addressCarry  = l_addressLow < g_drawRecordBufferAddress.x ? k_addressCarryValue : k_addressNoCarryValue;

    RWStructuredBuffer<ModelGPUDrivenIndirectCommand> l_indirectCommandBuffer = ResourceDescriptorHeap[g_indirectCommandBufferUAVDescriptorIndex];

    ModelGPUDrivenIndirectCommand l_indirectCommand = (ModelGPUDrivenIndirectCommand)k_zeroInitializeValue;

    l_indirectCommand.constantBufferAddress = uint2(l_addressLow, g_drawRecordBufferAddress.y + l_addressCarry);

    // 1つのAmplificationShaderGroupが32個のMeshletを担当するため、Group数は切り上げで求める
    // (「数 + 1Groupの数 - 1」を1Groupの数で割ると、余りがあるときだけ1つ増える)
    l_indirectCommand.dispatchMeshArguments = uint3((l_meshletCount + k_meshletCountPerAmplificationShaderGroup - k_countToLastIndexOffset) / k_meshletCountPerAmplificationShaderGroup,
                                                    k_dispatchMeshGroupCountY,
                                                    k_dispatchMeshGroupCountZ);

    l_indirectCommandBuffer[l_drawIndex] = l_indirectCommand;
}

// 1回目(Early)で隠れていると判定した組み合わせを、2回目で判定し直すために記録する
void AppendOcclusionCandidate(const uint a_instanceIndex, const uint a_meshInfoIndex)
{
    RWStructuredBuffer<uint> l_candidateCounterBuffer = ResourceDescriptorHeap[g_candidateCounterBufferUAVDescriptorIndex];

    uint l_candidateIndex = k_initialCounterValue;

    InterlockedAdd(l_candidateCounterBuffer[k_candidateCountCounterIndex], k_counterIncrementValue, l_candidateIndex);

    if (l_candidateIndex >= g_maxCandidateCount) { return; }

    RWStructuredBuffer<ModelGPUDrivenOcclusionCandidate> l_candidateBuffer = ResourceDescriptorHeap[g_candidateBufferUAVDescriptorIndex];

    ModelGPUDrivenOcclusionCandidate l_candidate = (ModelGPUDrivenOcclusionCandidate)k_zeroInitializeValue;

    l_candidate.instanceIndex = a_instanceIndex;
    l_candidate.meshInfoIndex = a_meshInfoIndex;

    l_candidateBuffer[l_candidateIndex] = l_candidate;
}

[numthreads(k_modelGPUDrivenCullingThreadCountX, k_modelGPUDrivenCullingThreadCountY, k_modelGPUDrivenCullingThreadCountZ)]
void main(const uint3 a_dispatchThreadID : SV_DispatchThreadID)
{
    uint l_instanceIndex = k_initialIndex;
    uint l_meshInfoIndex = k_initialIndex;

    if (g_cullingMode == k_cullingModeLate)
    {
        // 2回目 : 1回目で記録した候補だけを担当する
        RWStructuredBuffer<uint> l_candidateCounterBuffer = ResourceDescriptorHeap[g_candidateCounterBufferUAVDescriptorIndex];

        const uint l_candidateCount = min(l_candidateCounterBuffer[k_candidateCountCounterIndex], g_maxCandidateCount);

        if (a_dispatchThreadID.x >= l_candidateCount) { return; }

        RWStructuredBuffer<ModelGPUDrivenOcclusionCandidate> l_candidateBuffer = ResourceDescriptorHeap[g_candidateBufferUAVDescriptorIndex];

        const ModelGPUDrivenOcclusionCandidate l_candidate = l_candidateBuffer[a_dispatchThreadID.x];

        l_instanceIndex = l_candidate.instanceIndex;
        l_meshInfoIndex = l_candidate.meshInfoIndex;
    }
    else
    {
        // 通常 / 1回目 : 「インスタンス × Mesh」の組み合わせを1つずつ担当する
        const uint l_pairIndex = a_dispatchThreadID.x;

        // 最後のGroupには担当が無い余分なThreadが含まれる
        if (l_pairIndex >= g_instanceCount * g_meshCount) { return; }

        l_instanceIndex = g_instanceOffset + l_pairIndex / g_meshCount;
        l_meshInfoIndex = g_meshInfoOffset + l_pairIndex % g_meshCount;
    }

    StructuredBuffer<ModelGPUDrivenInstance> l_instanceBuffer = ResourceDescriptorHeap[g_instanceBufferSRVDescriptorIndex];
    StructuredBuffer<ModelGPUDrivenMeshInfo> l_meshInfoBuffer = ResourceDescriptorHeap[g_meshInfoBufferSRVDescriptorIndex];

    const ModelGPUDrivenInstance l_instance = l_instanceBuffer[l_instanceIndex];
    const ModelGPUDrivenMeshInfo l_meshInfo = l_meshInfoBuffer[l_meshInfoIndex];

    // Meshを囲む球をワールド座標へ変換して、視錐台に入っているか判定する
    const float3 l_worldCenter = mul(float4(l_meshInfo.boundsCenter, k_positionElementW), l_instance.worldMatrix).xyz;
    const float  l_worldRadius = l_meshInfo.boundsRadius * l_instance.worldMaxScale;

    if (!IsVisibleBoundingSphere(l_worldCenter, l_worldRadius))
    {
        // カリングの可視化中は、視錐台の外の物も「視錐台の外」の印を付けて描画する
        if (g_isCullingDebugVisualize == k_trueValue)
        {
            WriteDrawCommand(l_instance,
                             l_meshInfo,
                             l_worldCenter,
                             l_worldRadius,
                             k_modelCullingDebugStateMeshFrustumCulled);
        }

        return;
    }

    // 影の描画では、シャドウマップ上で1テクセルにも満たない小さな部品は影がほとんど見えないため描画しない
    if (g_isOrthographicLOD == k_trueValue &&
        l_worldRadius * k_diameterScale * g_lodProjectionScale < g_minProjectedSizeTexel)
    {
        return;
    }

    // 手前の物に隠れているか判定する
    if (IsOccludedBoundingSphere(l_worldCenter, l_worldRadius))
    {
        // カリングの可視化中は、隠れている物も「隠れている」の印を付けて描画する(2回目の候補にはしない)
        if (g_isCullingDebugVisualize == k_trueValue)
        {
            WriteDrawCommand(l_instance,
                             l_meshInfo,
                             l_worldCenter,
                             l_worldRadius,
                             k_modelCullingDebugStateMeshOccluded);

            return;
        }

        // 1回目で隠れていた物は、今のフレームでは見えている可能性があるため候補として残す
        // 2回目で隠れていた物は、本当に見えないので描画しない
        if (g_cullingMode == k_cullingModeEarly)
        {
            AppendOcclusionCandidate(l_instanceIndex, l_meshInfoIndex);
        }

        return;
    }

    WriteDrawCommand(l_instance,
                     l_meshInfo,
                     l_worldCenter,
                     l_worldRadius,
                     k_modelCullingDebugStateVisible);
}