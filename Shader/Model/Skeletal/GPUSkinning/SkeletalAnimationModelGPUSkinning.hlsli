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

// ベイクしたスキニング行列1つ分
// StructuredBufferの行列は、そのままだと列優先(column_major)として読まれるため、
// C++のMatrix(行優先)と同じ並びとして読めるようにrow_majorを指定した構造体で包む
struct GPUSkinningMatrixElement
{
    row_major float4x4 skinningMatrix;
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

// 頂点番号の頂点を読み、最大4本のボーンの影響を重み付きで合計して変形する
// (従来のComputeShader(SkeletalAnimationVertexSkinning_CS.hlsl)と同じ計算)
GPUSkinnedVertex LoadGPUSkinnedVertex(const uint a_vertexIndex)
{
    StructuredBuffer<SkeletalAnimationModelVertex> l_vertexBuffer         = ResourceDescriptorHeap[g_vertexBufferSRVDescriptorIndex];
    StructuredBuffer<GPUSkinningMatrixElement>     l_skinningMatrixBuffer = ResourceDescriptorHeap[g_skinningMatrixBufferSRVDescriptorIndex];

    const SkeletalAnimationModelVertex l_sourceVertex = l_vertexBuffer[a_vertexIndex];

    const uint4 l_bonePaletteIndexList = uint4(l_sourceVertex.bonePaletteIndexZero,
                                               l_sourceVertex.bonePaletteIndexOne,
                                               l_sourceVertex.bonePaletteIndexTwo,
                                               l_sourceVertex.bonePaletteIndexThree);

    const float4 l_boneWeightList = max(l_sourceVertex.boneWeight, k_gpuSkinningMINBoneWeight);

    float3 l_accumulatedPosition = float3(0.0F, 0.0F, 0.0F);
    float3 l_accumulatedNormal   = float3(0.0F, 0.0F, 0.0F);
    float3 l_accumulatedTangent  = float3(0.0F, 0.0F, 0.0F);
    float  l_totalBoneWeight     = 0.0F;

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

    GPUSkinnedVertex l_skinnedVertex = (GPUSkinnedVertex)0;

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