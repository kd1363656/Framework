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
}

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    
}