#ifndef MODEL_SKELETAL_ANIMATION_MODEL_GPU_SKINNING_HLSLI
#define MODEL_SKELETAL_ANIMATION_MODEL_GPU_SKINNING_HLSLI
#include "../SkeletalAnimationModel.hlsli"

// メッシュシェーダーの中で頂点をスキニング(ボーンに合わせて変形)するための関数
//
// 従来はComputeShaderで全頂点を変形し、変形後の頂点を別のBufferへ書き出してから描画していた。
// ここでは描画するMeshletの頂点だけを、メッシュシェーダーがその場で変形する。
// スキニング行列は事前にベイクしてあるため、ボーンの計算はせず行列を読むだけで済む。
//
// ※ Model.hlsliのcbuffer CBModelPerObject(g_skinningMatrixBufferSRVDescriptorIndexなど)を使うため、
//    Model.hlsliを先にincludeしておくこと

static const uint  k_gpuSkinningMAXBoneInfluenceCount = 4U;
static const float k_gpuSkinningBoneWeightEpsilon     = 0.000001F;
static const float k_gpuSkinningMINBoneWeight         = 0.0F;
static const float k_gpuSkinningPositionElementW      = 1.0F;
static const float k_gpuSkinningDirectionElementW     = 0.0F;

// C++側のDescriptorHeap::k_invalidDescriptorIndexと同じ値(無効なDescriptorの番号)
static const uint k_gpuSkinningInvalidDescriptorIndex = 0xFFFFFFFFU;

// halfを2つ詰めたuintから、1つ目(下位16bit)と2つ目(上位16bit)を取り出すための値
static const uint k_gpuSkinningLowerHalfMask      = 0xFFFFU;
static const uint k_gpuSkinningUpperHalfShiftBits = 16U;

static const float k_gpuSkinningEmptyMorphWeight = 0.0F;

// 影響するボーンの値を足し合わせる前の初期値
static const float3 k_gpuSkinningInitialAccumulatedVector = float3(0.0F, 0.0F, 0.0F);
static const float  k_gpuSkinningInitialTotalBoneWeight   = 0.0F;

// 構造体を全て0で初期化するときに使う値
static const uint k_gpuSkinningZeroInitializeValue = 0U;

// ベイクしたスキニング行列1つ分
// StructuredBufferの行列は、そのままだと列優先(column_major)として読まれるため、
// C++のMatrix(行優先)と同じ並びとして読めるようにrow_majorを指定した構造体で包む
struct GPUSkinningMatrixElement
{
    row_major float4x4 skinningMatrix;
};

// 頂点1つの、モーフの移動量の範囲(C++のStruct::ModelMorphVertexRangeと同じ並び)
struct GPUSkinningMorphVertexRange
{
    uint deltaOffset;
    uint deltaCount;
};

// 頂点1つが受けるモーフのチャンネル1つ分の移動量(C++のStruct::SBModelMorphVertexDeltaと同じ並び)
// 読む量を減らすため、移動量はhalf(16bitの浮動小数点数)を2つずつuintへ詰めている
struct GPUSkinningMorphVertexDelta
{
    uint deltaPositionXY;
    uint deltaPositionZNormalX;
    uint deltaNormalYZ;
    uint channelIndex;
};

// スキニング後の頂点
struct GPUSkinnedVertex
{
    float3 position;
    float3 normal;
    float4 tangent;
    float2 uv;
};

// 向きのベクトルの長さを1にする(長さがほぼ0の場合は元の向きを使う)
float3 NormalizeGPUSkinnedDirection(const float3 a_direction, const float3 a_fallbackDirection)
{
    const float l_lengthSquared = dot(a_direction, a_direction);

    if (l_lengthSquared <= k_gpuSkinningBoneWeightEpsilon) { return a_fallbackDirection; }

    return a_direction * rsqrt(l_lengthSquared);
}

// halfを2つ詰めたuintから、1つ目(下位16bit)を取り出してfloatにする
// f16tof32 : uintの下位16bitをhalfとして読み、floatへ変換するHLSLの関数
float UnpackGPUSkinningLowerHalf(const uint a_packedHalf2)
{
    return f16tof32(a_packedHalf2 & k_gpuSkinningLowerHalfMask);
}

// halfを2つ詰めたuintから、2つ目(上位16bit)を取り出してfloatにする
float UnpackGPUSkinningUpperHalf(const uint a_packedHalf2)
{
    return f16tof32(a_packedHalf2 >> k_gpuSkinningUpperHalfShiftBits);
}

// 頂点にモーフを足す(元の位置 + 移動量 × ウェイト を、効いている全てのチャンネルについて合計する)
// モーフを持たないMeshや、ウェイトが無い場合は何もしない
// ※ スキニングはモーフした後の形に対して行う(表情を付けた顔が、首の動きに合わせて動く)
void ApplyGPUSkinningMorph(const uint a_vertexIndex, inout float3 a_position, inout float3 a_normal)
{
    if (g_morphDeltaBufferSRVDescriptorIndex  == k_gpuSkinningInvalidDescriptorIndex ||
        g_morphWeightBufferSRVDescriptorIndex == k_gpuSkinningInvalidDescriptorIndex)
    {
        return;
    }

    StructuredBuffer<GPUSkinningMorphVertexRange> l_morphVertexRangeBuffer = ResourceDescriptorHeap[g_morphVertexRangeBufferSRVDescriptorIndex];
    StructuredBuffer<GPUSkinningMorphVertexDelta> l_morphDeltaBuffer       = ResourceDescriptorHeap[g_morphDeltaBufferSRVDescriptorIndex];
    StructuredBuffer<float>                       l_morphWeightBuffer      = ResourceDescriptorHeap[g_morphWeightBufferSRVDescriptorIndex];

    // この頂点の移動量が、移動量リストの何番目から何個あるか(影響を受けない頂点は0個)
    const GPUSkinningMorphVertexRange l_morphVertexRange = l_morphVertexRangeBuffer[a_vertexIndex];

    for (uint l_deltaIndex = l_morphVertexRange.deltaOffset; l_deltaIndex < l_morphVertexRange.deltaOffset + l_morphVertexRange.deltaCount; ++l_deltaIndex)
    {
        const GPUSkinningMorphVertexDelta l_morphDelta = l_morphDeltaBuffer[l_deltaIndex];

        const float l_morphWeight = l_morphWeightBuffer[g_morphWeightOffset + l_morphDelta.channelIndex];

        // ウェイトが0のチャンネルは形を変えないので読み飛ばす(表情を付けていないときは、ほぼ全て読み飛ばされる)
        if (l_morphWeight == k_gpuSkinningEmptyMorphWeight) { continue; }

        const float3 l_deltaPosition = float3(UnpackGPUSkinningLowerHalf(l_morphDelta.deltaPositionXY),
                                              UnpackGPUSkinningUpperHalf(l_morphDelta.deltaPositionXY),
                                              UnpackGPUSkinningLowerHalf(l_morphDelta.deltaPositionZNormalX));

        const float3 l_deltaNormal = float3(UnpackGPUSkinningUpperHalf(l_morphDelta.deltaPositionZNormalX),
                                            UnpackGPUSkinningLowerHalf(l_morphDelta.deltaNormalYZ),
                                            UnpackGPUSkinningUpperHalf(l_morphDelta.deltaNormalYZ));

        a_position += l_deltaPosition * l_morphWeight;
        a_normal   += l_deltaNormal   * l_morphWeight;
    }
}

// 頂点番号の頂点を読み、最大4本のボーンの影響を重み付きで合計して変形する
// (従来のComputeShader(SkeletalAnimationVertexSkinning_CS.hlsl)と同じ計算)
GPUSkinnedVertex LoadGPUSkinnedVertex(const uint a_vertexIndex)
{
    StructuredBuffer<SkeletalAnimationModelVertex> l_vertexBuffer         = ResourceDescriptorHeap[g_vertexBufferSRVDescriptorIndex];
    StructuredBuffer<GPUSkinningMatrixElement>     l_skinningMatrixBuffer = ResourceDescriptorHeap[g_skinningMatrixBufferSRVDescriptorIndex];

    SkeletalAnimationModelVertex l_sourceVertex = l_vertexBuffer[a_vertexIndex];

    // スキニングの前にモーフを足す
    // 法線が変わると接線と直角でなくなるため、接線から法線の向きの成分を取り除いて直角に戻す(グラム・シュミットの直交化)
    const float3 l_originalNormal = l_sourceVertex.normal;

    ApplyGPUSkinningMorph(a_vertexIndex, l_sourceVertex.position, l_sourceVertex.normal);

    l_sourceVertex.normal      = NormalizeGPUSkinnedDirection(l_sourceVertex.normal, l_originalNormal);
    l_sourceVertex.tangent.xyz = NormalizeGPUSkinnedDirection(l_sourceVertex.tangent.xyz - l_sourceVertex.normal * dot(l_sourceVertex.normal, l_sourceVertex.tangent.xyz), l_sourceVertex.tangent.xyz);

    const uint4 l_bonePaletteIndexList = uint4(l_sourceVertex.bonePaletteIndexZero,
                                               l_sourceVertex.bonePaletteIndexOne,
                                               l_sourceVertex.bonePaletteIndexTwo,
                                               l_sourceVertex.bonePaletteIndexThree);

    const float4 l_boneWeightList = max(l_sourceVertex.boneWeight, k_gpuSkinningMINBoneWeight);

    float3 l_accumulatedPosition = k_gpuSkinningInitialAccumulatedVector;
    float3 l_accumulatedNormal   = k_gpuSkinningInitialAccumulatedVector;
    float3 l_accumulatedTangent  = k_gpuSkinningInitialAccumulatedVector;
    float  l_totalBoneWeight     = k_gpuSkinningInitialTotalBoneWeight;

    [unroll]
    for (uint l_influenceIndex = 0U; l_influenceIndex < k_gpuSkinningMAXBoneInfluenceCount; ++l_influenceIndex)
    {
        const float l_boneWeight = l_boneWeightList[l_influenceIndex];

        if (l_boneWeight <= k_gpuSkinningBoneWeightEpsilon) { continue; }

        // このMesh、このコマのスキニング行列の先頭 + ボーンパレット番号
        // スキニング行列は C++のMatrix(行優先)をそのまま並べているため、row_majorとして掛け算する
        const float4x4 l_skinningMatrix = l_skinningMatrixBuffer[g_skinningMatrixOffset + l_bonePaletteIndexList[l_influenceIndex]].skinningMatrix;

        l_accumulatedPosition += mul(float4(l_sourceVertex.position,    k_gpuSkinningPositionElementW),  l_skinningMatrix).xyz * l_boneWeight;
        l_accumulatedNormal   += mul(float4(l_sourceVertex.normal,      k_gpuSkinningDirectionElementW), l_skinningMatrix).xyz * l_boneWeight;
        l_accumulatedTangent  += mul(float4(l_sourceVertex.tangent.xyz, k_gpuSkinningDirectionElementW), l_skinningMatrix).xyz * l_boneWeight;
        l_totalBoneWeight     += l_boneWeight;
    }

    GPUSkinnedVertex l_skinnedVertex = (GPUSkinnedVertex)k_gpuSkinningZeroInitializeValue;

    l_skinnedVertex.uv = l_sourceVertex.uv;

    // ボーンの影響が無い頂点は、元の位置のまま使う
    if (l_totalBoneWeight <= k_gpuSkinningBoneWeightEpsilon)
    {
        l_skinnedVertex.position = l_sourceVertex.position;
        l_skinnedVertex.normal   = l_sourceVertex.normal;
        l_skinnedVertex.tangent  = l_sourceVertex.tangent;

        return l_skinnedVertex;
    }

    // 重みの合計が1でない場合に備えて、合計で割って平均にする
    const float l_inverseTotalBoneWeight = rcp(l_totalBoneWeight);

    l_skinnedVertex.position = l_accumulatedPosition * l_inverseTotalBoneWeight;
    l_skinnedVertex.normal   = NormalizeGPUSkinnedDirection(l_accumulatedNormal * l_inverseTotalBoneWeight, l_sourceVertex.normal);
    l_skinnedVertex.tangent  = float4(NormalizeGPUSkinnedDirection(l_accumulatedTangent * l_inverseTotalBoneWeight, l_sourceVertex.tangent.xyz), l_sourceVertex.tangent.w);

    return l_skinnedVertex;
}

#endif // MODEL_SKELETAL_ANIMATION_MODEL_GPU_SKINNING_HLSLI