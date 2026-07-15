#include "SkeletalAnimation.hlsli"

cbuffer CBSkeletalAnimationLocalMatrix : register(b0)
{
    float g_blendWeight;
    float g_animationTimeSecond;
    float g_blendTargetAnimationTimeSecond;
    
    uint g_boneBufferSRVDescriptorIndex;
    uint g_motionSequenceBufferSRVDescriptorIndex;
    uint g_boneMotionTrackBufferSRVDescriptorIndex;
    uint g_keyFrameBufferSRVDescriptorIndex;
    uint g_boneMatrixBufferUAVDescriptorIndex;
    
    uint g_motionIndex;
    uint g_blendTargetMotionIndex;
    uint g_boneCount;
    uint g_isBlending;
};

static const uint k_localMatrixThreadCountX = 64U;
static const uint k_localMatrixThreadCountY = 1U;
static const uint k_localMatrixThreadCountZ = 1U;

static const uint k_blendDisabled = 0U;

// 1Plyaerにつき1GroupをDispatchし、
// Group内の各Threadが64Bone間隔でLocalMatrixを計算する
[numthreads(k_localMatrixThreadCountX, k_localMatrixThreadCountY, k_localMatrixThreadCountZ)]
void main(const uint3 a_groupThreadID : SV_GroupThreadID)
{
    StructuredBuffer<SkeletalAnimationBoneBufferElement>            l_boneBuffer            = ResourceDescriptorHeap[g_boneBufferSRVDescriptorIndex];
    StructuredBuffer<SkeletalAnimationMotionSequenceBufferElement>  l_motionSequenceBuffer  = ResourceDescriptorHeap[g_motionSequenceBufferSRVDescriptorIndex];
    StructuredBuffer<SkeletalAnimationBoneMotionTrackBufferElement> l_boneMotionTrackBuffer = ResourceDescriptorHeap[g_boneMotionTrackBufferSRVDescriptorIndex];
    StructuredBuffer<SkeletalAnimationKeyFrameBufferElement>        l_keyFrameBuffer        = ResourceDescriptorHeap[g_keyFrameBufferSRVDescriptorIndex];
    StructuredBuffer<SkeletalAnimationBoneMatrixBufferElement>      l_boneMatrixBuffer      = ResourceDescriptorHeap[g_boneMatrixBufferUAVDescriptorIndex];
    
    // 各ThreadがThreadCountX間隔でBoneを担当する
    for (uint l_boneIndex = a_groupThreadID.x; l_boneIndex < g_boneCount; l_boneIndex += k_localMatrixThreadCountX)
    {
        const SkeletalAnimationBoneBufferElement l_bone = l_boneBuffer[l_boneIndex];
        
        // 現在Animationの再生時刻に対応するLocalSRTを取得する
        SkeletalAnimationLocalTransform l_localTrnsform = SampleAnimationLocalTransform(l_bone,
                                                                                        g_animationTimeSecond,
                                                                                        l_boneIndex,
                                                                                        g_motionIndex,
                                                                                        l_motionSequenceBuffer,
                                                                                        l_boneMotionTrackBuffer,
                                                                                        l_keyFrameBuffer);
        
        // Blend中だけBlend先AnimationのLocalSRTを取得し、
        // 現在Animationとの間を補間する
        if (g_isBlending != k_blendDisabled)
        {
            const SkeletalAnimationLocalTransform l_blendTargetLocalTransform = SampleAnimationLocalTransform(l_bone,
                                                                                                              g_blendTargetAnimationTimeSecond,
                                                                                                              l_boneIndex,
                                                                                                              g_blendTargetMotionIndex,
                                                                                                              l_motionSequenceBuffer,
                                                                                                              l_boneMotionTrackBuffer,
                                                                                                              l_keyFrameBuffer);

            l_localTrnsform = InterpolateAnimationLocalTransform(l_localTrnsform, l_blendTargetLocalTransform, g_blendWeight);
        }
        
        SkeletalAnimationBoneMatrixBufferElement l_boneMatrixBufferElement = (SkeletalAnimationBoneMatrixBufferElement)0;
        
        l_boneMatrixBufferElement.boneMatrix = CreateAnimationLocalMatrix(l_localTrnsform);

        l_boneMatrixBuffer[l_boneIndex] = l_boneMatrixBufferElement;
    }
}