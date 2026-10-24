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

cbuffer CBModelGPUDrivenCullingPass : register(b0)
{
    // 視錐台を作る平面(xyz : 平面の法線、w : 原点からの距離)
    float4 g_frustumPlaneList[6];

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

    uint  g_maxCandidateCount;
    uint  g_isOrthographicLOD;
    float g_minProjectedSizeTexel;
    uint  g_padding;
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

// CounterBufferの要素番号
static const uint k_drawCountCounterIndex      = 0U;
static const uint k_triangleCountCounterIndex  = 1U;
static const uint k_candidateCountCounterIndex = 2U;

// 1つのAmplificationShaderGroupが担当するMeshlet数(C++側と同じ)
static const uint k_meshletCountPerAmplificationShaderGroup = 32U;

static const uint k_dispatchMeshGroupCountY = 1U;
static const uint k_dispatchMeshGroupCountZ = 1U;

static const uint k_drawRecordByteSize = 256U;

static const uint k_trueValue = 1U;

// C++側のDescriptorHeap::k_invalidDescriptorIndexと同じ値(無効なDescriptorの番号)
static const uint k_invalidDescriptorIndex = 0xFFFFFFFFU;

static const uint k_firstLODIndex = 0U;

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
    float2 l_uvMIN        = float2( 1.0F,  1.0F);
    float2 l_uvMAX        = float2( 0.0F,  0.0F);
    float  l_nearestDepth = 1.0F;

    for (uint l_cornerIndex = 0U; l_cornerIndex < k_boundsCornerCount; ++l_cornerIndex)
    {
        // 角の番号のビット(1, 2, 4)で、x、y、zそれぞれを-側にするか+側にするかを決める
        const float3 l_cornerSign = float3((l_cornerIndex & 1U) ? 1.0F : -1.0F,
                                           (l_cornerIndex & 2U) ? 1.0F : -1.0F,
                                           (l_cornerIndex & 4U) ? 1.0F : -1.0F);

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
    const float2 l_sizeInTexel = (l_uvMAX - l_uvMIN) * g_hierarchicalDepthSize;
    const float  l_mipLevel    = clamp(ceil(log2(max(max(l_sizeInTexel.x, l_sizeInTexel.y), 1.0F))), 0.0F, (float)(g_hierarchicalDepthMIPCount - 1U));

    Texture2D<float> l_hierarchicalDepthTexture = ResourceDescriptorHeap[g_hierarchicalDepthTextureSRVDescriptorIndex];

    const uint  l_mip     = (uint)l_mipLevel;
    const uint2 l_mipSize = max(uint2(g_hierarchicalDepthSize) >> l_mip, uint2(1U, 1U));
    const uint2 l_texelMIN = min(uint2(l_uvMIN * float2(l_mipSize)), l_mipSize - 1U);
    const uint2 l_texelMAX = min(uint2(l_uvMAX * float2(l_mipSize)), l_mipSize - 1U);

    // 3. 範囲の四隅のピクセルから、一番奥の深度を求める
    const float l_depth0 = l_hierarchicalDepthTexture.Load(int3(l_texelMIN.x, l_texelMIN.y, l_mip));
    const float l_depth1 = l_hierarchicalDepthTexture.Load(int3(l_texelMAX.x, l_texelMIN.y, l_mip));
    const float l_depth2 = l_hierarchicalDepthTexture.Load(int3(l_texelMIN.x, l_texelMAX.y, l_mip));
    const float l_depth3 = l_hierarchicalDepthTexture.Load(int3(l_texelMAX.x, l_texelMAX.y, l_mip));

    const float l_farthestDepth = max(max(l_depth0, l_depth1), max(l_depth2, l_depth3));

    // 4. 物体の一番手前でさえ、範囲内の一番奥より奥にあるなら、完全に隠れている
    return l_nearestDepth > l_farthestDepth + k_occlusionDepthBias;
}

// 画面上の誤差が許容値以下になる、最も三角形の少ないLODを選ぶ
// (C++側のModelLODSelectorと同じ計算)
ModelGPUDrivenMeshLOD SelectMeshLOD(const ModelGPUDrivenMeshInfo a_meshInfo, const float3 a_worldCenter, const float a_worldRadius, const float a_worldMaxScale)
{
    StructuredBuffer<ModelGPUDrivenMeshLOD> l_lodBuffer = ResourceDescriptorHeap[g_lodBufferSRVDescriptorIndex];

    // カメラから球の表面までの距離
    // 影(平行投影)では距離によって大きさが変わらないため、距離を1として扱う
    // (g_lodProjectionScaleには「1メートルがシャドウマップの何テクセルか」が入っている)
    const float l_distance = g_isOrthographicLOD == k_trueValue ? k_orthographicLODDistance : max(distance(g_cameraWorldPosition, a_worldCenter) - a_worldRadius, g_lodMinDistance);

    // 三角形が一番少ないLODから順に調べる
    for (uint l_lodIndex = a_meshInfo.lodCount; l_lodIndex > k_firstLODIndex; --l_lodIndex)
    {
        const ModelGPUDrivenMeshLOD l_lod = l_lodBuffer[a_meshInfo.lodOffset + l_lodIndex - 1U];

        const float l_screenSpaceErrorPixel = l_lod.simplifyError * a_worldMaxScale * g_lodProjectionScale / l_distance;

        if (l_screenSpaceErrorPixel <= g_lodMaxScreenSpaceErrorPixel) { return l_lod; }
    }

    // どのLODも許容値を超える場合は元の形(LOD0)を使う
    return l_lodBuffer[a_meshInfo.lodOffset];
}

// 描画1回分の定数バッファと描画コマンドを書き込む
void WriteDrawCommand(const ModelGPUDrivenInstance a_instance, const ModelGPUDrivenMeshInfo a_meshInfo, const float3 a_worldCenter, const float a_worldRadius)
{
    // LODを選ぶ(LODが無いMeshは全Meshletを描画する)
    uint l_meshletOffset = 0U;
    uint l_meshletCount  = a_meshInfo.allMeshletCount;
    uint l_triangleCount = a_meshInfo.allTriangleCount;

    if (a_meshInfo.lodCount > k_firstLODIndex)
    {
        const ModelGPUDrivenMeshLOD l_lod = SelectMeshLOD(a_meshInfo, a_worldCenter, a_worldRadius, a_instance.worldMaxScale);

        l_meshletOffset = l_lod.meshletOffset;
        l_meshletCount  = l_lod.meshletCount;
        l_triangleCount = l_lod.triangleCount;
    }

    // 描画コマンドの書き込み先を1つ確保する
    // InterlockedAddは複数Threadが同時に足しても正しく数えられる加算で、
    // 第三引数に「加算前の値」が入るため、それを自分専用の書き込み位置として使う
    RWStructuredBuffer<uint> l_counterBuffer = ResourceDescriptorHeap[g_counterBufferUAVDescriptorIndex];

    uint l_drawIndex = 0U;

    InterlockedAdd(l_counterBuffer[k_drawCountCounterIndex], 1U, l_drawIndex);

    // 用意したBufferの数を超えた分は描画しない
    // (ExecuteIndirectもC++側で最大数を指定しているため、数が超えても範囲外は読まれない)
    if (l_drawIndex >= g_maxDrawCount) { return; }

    InterlockedAdd(l_counterBuffer[k_triangleCountCounterIndex], l_triangleCount);

    // 描画1回分の定数バッファを書き込む
    RWStructuredBuffer<ModelGPUDrivenDrawRecord> l_drawRecordBuffer = ResourceDescriptorHeap[g_drawRecordBufferUAVDescriptorIndex];

    ModelGPUDrivenDrawRecord l_drawRecord = (ModelGPUDrivenDrawRecord)0;

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

    // スキニングするモデルなら、このインスタンスのアニメーションのコマに対応するスキニング行列の位置を求める
    if (a_meshInfo.skinningMatrixBufferSRVDescriptorIndex != k_invalidDescriptorIndex &&
        a_meshInfo.skinningFrameCount > 0U)
    {
        const uint l_frameIndex = a_instance.animationFrameIndex % a_meshInfo.skinningFrameCount;

        l_drawRecord.skinningMatrixBufferSRVDescriptorIndex = a_meshInfo.skinningMatrixBufferSRVDescriptorIndex;
        l_drawRecord.skinningMatrixOffset                   = l_frameIndex * a_meshInfo.skinningPaletteStride + a_meshInfo.skinningPaletteBase;
        l_drawRecord.isGPUSkinning                          = k_trueValue;
    }

    l_drawRecordBuffer[l_drawIndex] = l_drawRecord;

    // 描画コマンドを書き込む
    // 定数バッファのアドレス = DrawRecordBufferの先頭アドレス + 番号 × 256byte
    // GPUのアドレスは64bitなので、下位32bitと上位32bitに分けて扱い、繰り上がりも計算する
    const uint l_addressOffset = l_drawIndex * k_drawRecordByteSize;
    const uint l_addressLow    = g_drawRecordBufferAddress.x + l_addressOffset;
    const uint l_addressCarry  = l_addressLow < g_drawRecordBufferAddress.x ? 1U : 0U;

    RWStructuredBuffer<ModelGPUDrivenIndirectCommand> l_indirectCommandBuffer = ResourceDescriptorHeap[g_indirectCommandBufferUAVDescriptorIndex];

    ModelGPUDrivenIndirectCommand l_indirectCommand = (ModelGPUDrivenIndirectCommand)0;

    l_indirectCommand.constantBufferAddress = uint2(l_addressLow, g_drawRecordBufferAddress.y + l_addressCarry);

    // 1つのAmplificationShaderGroupが32個のMeshletを担当するため、Group数は切り上げで求める
    l_indirectCommand.dispatchMeshArguments = uint3((l_meshletCount + k_meshletCountPerAmplificationShaderGroup - 1U) / k_meshletCountPerAmplificationShaderGroup,
                                                    k_dispatchMeshGroupCountY,
                                                    k_dispatchMeshGroupCountZ);

    l_indirectCommandBuffer[l_drawIndex] = l_indirectCommand;
}

// 1回目(Early)で隠れていると判定した組み合わせを、2回目で判定し直すために記録する
void AppendOcclusionCandidate(const uint a_instanceIndex, const uint a_meshInfoIndex)
{
    RWStructuredBuffer<uint> l_candidateCounterBuffer = ResourceDescriptorHeap[g_candidateCounterBufferUAVDescriptorIndex];

    uint l_candidateIndex = 0U;

    InterlockedAdd(l_candidateCounterBuffer[k_candidateCountCounterIndex], 1U, l_candidateIndex);

    if (l_candidateIndex >= g_maxCandidateCount) { return; }

    RWStructuredBuffer<ModelGPUDrivenOcclusionCandidate> l_candidateBuffer = ResourceDescriptorHeap[g_candidateBufferUAVDescriptorIndex];

    ModelGPUDrivenOcclusionCandidate l_candidate = (ModelGPUDrivenOcclusionCandidate)0;

    l_candidate.instanceIndex = a_instanceIndex;
    l_candidate.meshInfoIndex = a_meshInfoIndex;

    l_candidateBuffer[l_candidateIndex] = l_candidate;
}

[numthreads(k_modelGPUDrivenCullingThreadCountX, k_modelGPUDrivenCullingThreadCountY, k_modelGPUDrivenCullingThreadCountZ)]
void main(const uint3 a_dispatchThreadID : SV_DispatchThreadID)
{
    uint l_instanceIndex = 0U;
    uint l_meshInfoIndex = 0U;

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

    if (!IsVisibleBoundingSphere(l_worldCenter, l_worldRadius)) { return; }

    // 影の描画では、シャドウマップ上で1テクセルにも満たない小さな部品は影がほとんど見えないため描画しない
    if (g_isOrthographicLOD == k_trueValue &&
        l_worldRadius * k_diameterScale * g_lodProjectionScale < g_minProjectedSizeTexel)
    {
        return;
    }

    // 手前の物に隠れているか判定する
    if (IsOccludedBoundingSphere(l_worldCenter, l_worldRadius))
    {
        // 1回目で隠れていた物は、今のフレームでは見えている可能性があるため候補として残す
        // 2回目で隠れていた物は、本当に見えないので描画しない
        if (g_cullingMode == k_cullingModeEarly)
        {
            AppendOcclusionCandidate(l_instanceIndex, l_meshInfoIndex);
        }

        return;
    }

    WriteDrawCommand(l_instance, l_meshInfo, l_worldCenter, l_worldRadius);
}