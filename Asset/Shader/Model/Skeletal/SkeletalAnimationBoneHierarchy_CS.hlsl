#include "SkeletalAnimation.hlsli"

cbuffer CBSkeletalAnimationBoneHierarchy : register(b0)
{
    uint g_boneBufferSRVDescriptorIndex;
    uint g_boneMatrixBufferUAVDescriptorIndex;
    uint g_hierarchyDepth;
    uint g_boneCount;
}

static const uint k_boneHierarchyThreadCountX = 64U;
static const uint k_boneHierarchyThreadCountY = 1U;
static const uint k_boneHierarchyThreadCountZ = 1U;

// 同じHierarchyDepthのBonehあ互いを親に持たないため、
// 同一Dispatch内で並列にGlobalMatrixへ変換できる
[numthreads(k_boneHierarchyThreadCountX, k_boneHierarchyThreadCountY, k_boneHierarchyThreadCountZ)]
void main(const uint3 a_groupThreadID : SV_GroupThreadID)
{
    StructuredBuffer  <SkeletalAnimationBoneBufferElement>       l_boneBuffer       = ResourceDescriptorHeap[g_boneBufferSRVDescriptorIndex];
    RWStructuredBuffer<SkeletalAnimationBoneMatrixBufferElement> l_boneMatrixBuffer = ResourceDescriptorHeap[g_boneMatrixBufferUAVDescriptorIndex];
    
    for (uint l_boneIndex = a_groupThreadID.x; l_boneIndex < g_boneCount; l_boneIndex += k_boneHierarchyThreadCountX)
    {
        const SkeletalAnimationBoneBufferElement l_bone = l_boneBuffer[l_boneIndex];
        
        // 現在処理中のHierarchyDepthに所属しないBoneは処理しない
        if (l_bone.hierarchyDepth != g_hierarchyDepth) { continue; }

        SkeletalAnimationBoneMatrixBufferElement l_boneMatrixElement = l_boneMatrixBuffer[l_boneIndex];
        
        const SkeletalAnimationBoneMatrixBufferElement l_parentBoneMatrixElement = l_boneMatrixBuffer[l_bone.parentBoneIndex];
        
        // このプロジェクトは行ベクトル方式なので、
        // GlobalMatrix = LocalMatrix * ParentGlobalMatrix
        l_boneMatrixElement.boneMatrix = mul(l_boneMatrixElement.boneMatrix, l_parentBoneMatrixElement.boneMatrix);
        
        l_boneMatrixBuffer[l_boneIndex] = l_boneMatrixElement;
    }
}