// SkeletonのBone1個分の共有データ
struct SkeletalAnimationBoneBufferElement
{
    float3 bindPoseLocalScale;
    float4 bindPoseLocalRotation;
    float3 bindPoseLocalPosition;
    uint   parentBoneIndex;
    uint   hierarchyDepth;
};

// Motion1個分の共有データ
struct SkeletalAnimationMotionSequenceBufferElement
{
    float durationSecond;
    
    float frameRate;
    
    uint firstBoneMotionTrackIndex;
};

// Motion内にあるBone1個分のTrack情報
struct SkeletalAnimationBoneMotionTrackBufferElement
{
    uint firstKeyFrameIndex;
    
    uint keyFrameCount;
};