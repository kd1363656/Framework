// SkeletonのBone1個分の共有データ
struct SkeletalAnimationBoneBufferElement
{
    float3 bindPoseLocalScale;
    float4 bindPoseLocalRotation;
    float3 bindPoseLocalTranslation;
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

// KeyFrame1個分のSRTと再生時刻
struct SkeletalAnimationKeyFrameBufferElement
{
    float3 scale;
    float4 rotation;
    float3 translation;
    float  timeSecond;
};

// ComputeShader内部で扱うBoneのLocalSRT
struct SkeletalAnimationLocalTransform
{
    float3 scale;
    float4 rotation;
    float3 translation;
};

// BoneMatrixBufferの1要素
struct SkeletalAnimationBoneMatrixBufferElement
{
    row_major float4x4 boneMatrix;
};

static const uint k_emptyKeyFrameCount  = 0U;
static const uint k_singleKeyFrameCount = 1U;

static const uint k_firstKeyFrameOffset = 0U;
static const uint k_nextKeyFrameOffset  = 1U;

static const uint k_binarySearchPartitionCount = 2U;

static const float4 k_identityQuaternion = float4(0.0F, 0.0F, 0.0F, 1.0F);

static const float k_shortestPathQuaternionDotThreshold = 0.0F;

static const float k_minTimeRange = 0.000001F;

static const float k_minQuaternionLengthSquared = 0.00000001F;
static const float k_minQuaternionSin           = 0.000001F;

static const float k_quaternionLerpThreshold = 0.9995F;

static const float k_completeBlendWeight = 1.0F;

static const float k_quaternionRotationMatrixBaseCoefficient   = 1.0F;
static const float k_quaternionRotationMatrixDoubleCoefficient = 2.0F;

static const float k_affineMatrixZeroElement        = 0.0F;
static const float k_affineMatrixHomogeneousElement = 1.0F;

// BoneのBindPoseからLocalSRTを取得する
SkeletalAnimationLocalTransform FetchBindPoseLocalTransform(const SkeletalAnimationBoneBufferElement a_bone)
{
    SkeletalAnimationLocalTransform l_localTransform;
    
    l_localTransform.scale       = a_bone.bindPoseLocalScale;
    l_localTransform.rotation    = a_bone.bindPoseLocalRotation;
    l_localTransform.translation = a_bone.bindPoseLocalTranslation;
    
    return l_localTransform;
}

// KeyFrameからLocalSRTを取得する
SkeletalAnimationLocalTransform FetchKeyFrameLocalTransform(const SkeletalAnimationKeyFrameBufferElement a_keyFrame)
{
    SkeletalAnimationLocalTransform l_localTransform;
    
    l_localTransform.scale       = a_keyFrame.scale;
    l_localTransform.rotation    = a_keyFrame.rotation;
    l_localTransform.translation = a_keyFrame.translation;
    
    return l_localTransform;
}

// Quaternionを正規化する
float4 NormalizeAnimationQuaternion(const float4 a_quaternion)
{
    const float l_lengthSquared = dot(a_quaternion, a_quaternion);
    
   // 長さがほぼ0の場合は正規化できないため、
    // 回転なしを表すIdentityQuaternionを返す
    if (l_lengthSquared <= k_minQuaternionLengthSquared)
    {
        return k_identityQuaternion;
    }

    return a_quaternion * rsqrt(l_lengthSquared);
}

// 2個のQuaternion間を最短経路で補間する
float4 SlerpAnimationQuaternion(const float4 a_startQuaternion, const float4 a_endQuaternion, const float a_blendWeight)
{
    const float4 l_startQuaternion = NormalizeAnimationQuaternion(a_startQuaternion);
    
    float4 l_endQuaternion = NormalizeAnimationQuaternion(a_endQuaternion);
    
    float l_quaternionDot = dot(l_startQuaternion, l_endQuaternion);
    
    // Quaternionのqと-qは同じ回転を表す
    // 内積が負の場合は終点側を反転し、
    // より短い回転経路で補間する
    if (l_quaternionDot < k_shortestPathQuaternionDotThreshold)
    {
        l_endQuaternion = -l_endQuaternion;
        l_quaternionDot = -l_quaternionDot;
    }
    
    l_quaternionDot = saturate(l_quaternionDot);

    const float l_blendWeight = saturate(a_blendWeight);
    
    // 2個のQuaternionがほぼ同じ場合は、
    // SlerpよりLerpして正規化した方が数値的に安定する
    if (l_quaternionDot >= k_quaternionLerpThreshold) { return NormalizeAnimationQuaternion(lerp(l_startQuaternion, l_endQuaternion, l_blendWeight)); }

    const float l_angle    = acos(l_quaternionDot);
    const float l_sinAngle = sin(l_angle);
    
    if (abs(l_sinAngle) <= k_minQuaternionSin) { return l_startQuaternion; }

    const float l_startWeight = sin((k_completeBlendWeight   - l_blendWeight) * l_angle) / l_sinAngle;
    const float l_endWeight   = sin(l_blendWeight * l_angle) / l_sinAngle;

    return NormalizeAnimationQuaternion(l_startQuaternion * l_startWeight + l_endQuaternion * l_endWeight);
}

// 2個のLocalSRT間を補間する
SkeletalAnimationLocalTransform InterpolateAnimationLocalTransform(const SkeletalAnimationLocalTransform a_startLocalTransform, const SkeletalAnimationLocalTransform a_endLocalTransform, const float a_blendWeight)
{
    const float l_blendWeight = saturate(a_blendWeight);
    
    SkeletalAnimationLocalTransform l_localTransform = (SkeletalAnimationLocalTransform)0;
    
    l_localTransform.scale       = lerp                    (a_startLocalTransform.scale,       a_endLocalTransform.scale,       l_blendWeight);
    l_localTransform.rotation    = SlerpAnimationQuaternion(a_startLocalTransform.rotation,    a_endLocalTransform.rotation,    l_blendWeight);
    l_localTransform.translation = lerp                    (a_startLocalTransform.translation, a_endLocalTransform.translation, l_blendWeight);

    return l_localTransform;
}

// 指定したMotion、Bone、再生時刻に対応するLocalSRTを取得する
SkeletalAnimationLocalTransform SampleAnimationLocalTransform(const SkeletalAnimationBoneBufferElement                              a_bone,
                                                              const float                                                           a_timeSecond,
                                                              const uint                                                            a_boneIndex,
                                                              const uint                                                            a_motionIndex,
                                                                    StructuredBuffer<SkeletalAnimationMotionSequenceBufferElement>  a_motionSequenceBuffer,
                                                                    StructuredBuffer<SkeletalAnimationBoneMotionTrackBufferElement> a_boneMotionTrackBuffer,
                                                                    StructuredBuffer<SkeletalAnimationKeyFrameBufferElement> a_keyFrameBuffer)
{
    const SkeletalAnimationMotionSequenceBufferElement l_motionSequence = a_motionSequenceBuffer[a_motionIndex];
    
    // MotionごとにBone数分のTrackが連続して格納されているため、
    // Motionの先頭TrackIndexへBoneIndexを加算して取得する
    const uint l_boneMotionTrackIndex = l_motionSequence.firstBoneMotionTrackIndex + a_boneIndex;

    const SkeletalAnimationBoneMotionTrackBufferElement l_boneMotionTrack = a_boneMotionTrackBuffer[l_boneMotionTrackIndex];
    
    // KeyFrameを持たないBoneはBindPoseを使用する
    if (l_boneMotionTrack.keyFrameCount == k_emptyKeyFrameCount) { return FetchBindPoseLocalTransform(a_bone); }

    const uint                                   l_firstKeyFrameIndex = l_boneMotionTrack.firstKeyFrameIndex;    
    const SkeletalAnimationKeyFrameBufferElement l_firstKeyFrame      = a_keyFrameBuffer[l_firstKeyFrameIndex];

    // KeyFrameが1個だけの場合は補間しない
    if (l_boneMotionTrack.keyFrameCount == k_singleKeyFrameCount) { return FetchKeyFrameLocalTransform(l_firstKeyFrame); }

    const uint l_lastKeyFrameOffset = l_boneMotionTrack.keyFrameCount - k_nextKeyFrameOffset;
    const uint l_lastKeyFrameIndex  = l_firstKeyFrameIndex            + l_lastKeyFrameOffset;
    
    const SkeletalAnimationKeyFrameBufferElement l_lastKeyFrame = a_keyFrameBuffer[l_lastKeyFrameIndex];
    
    // 最初のKeyFrame以前なら最初のPoseを使用する
    if (a_timeSecond <= l_firstKeyFrame.timeSecond) { return FetchKeyFrameLocalTransform(l_firstKeyFrame); }
    
    // 最後のKeyFrame以降なら最後のPoseを使用する
    if (a_timeSecond >= l_lastKeyFrame.timeSecond) { return FetchKeyFrameLocalTransform(l_lastKeyFrame); }
    
    uint l_previousKeyFrameOffset = k_firstKeyFrameOffset;
    uint l_nextKeyFrameOffset     = l_lastKeyFrameOffset;
    
    // KeyFrameは時刻順に格納されているため、
    // 現在時刻を挟む2個のKeyFrameを二分探索する
    [loop]
    while (l_nextKeyFrameOffset - l_previousKeyFrameOffset > k_nextKeyFrameOffset)
    {
        const uint                                   l_middleKeyFrameOffset = l_previousKeyFrameOffset + (l_nextKeyFrameOffset - l_previousKeyFrameOffset) / k_binarySearchPartitionCount;
        const uint                                   l_middleKeyFrameIndex  = l_firstKeyFrameIndex     + l_middleKeyFrameOffset;
        const SkeletalAnimationKeyFrameBufferElement l_middleKeyFrame       = a_keyFrameBuffer[l_middleKeyFrameIndex];
        
        if (l_middleKeyFrame.timeSecond <= a_timeSecond)
        {
            l_previousKeyFrameOffset = l_middleKeyFrameOffset;
        }
        else
        {
            l_nextKeyFrameOffset = l_middleKeyFrameOffset;
        }
    }
    
    const uint l_previousKeyFrameIndex = l_firstKeyFrameIndex + l_previousKeyFrameOffset;
    const uint l_nextKeyFrameIndex     = l_firstKeyFrameIndex + l_nextKeyFrameOffset;

    const SkeletalAnimationKeyFrameBufferElement l_previousKeyFrame = a_keyFrameBuffer[l_previousKeyFrameIndex];
    const SkeletalAnimationKeyFrameBufferElement l_nextKeyFrame     = a_keyFrameBuffer[l_nextKeyFrameIndex];
    
    const float l_keyFrameTimeRange = l_nextKeyFrame.timeSecond - l_previousKeyFrame.timeSecond;
    
    // 同時刻のKeyFrameが連続している場合は、
    // 0除算を避けて後側のKeyFrameを使用する
    if (l_keyFrameTimeRange <= k_minTimeRange) { return FetchKeyFrameLocalTransform(l_nextKeyFrame); }
    
    const float l_interpolationWeight = saturate((a_timeSecond - l_previousKeyFrame.timeSecond) / l_keyFrameTimeRange);
    
    return InterpolateAnimationLocalTransform(FetchKeyFrameLocalTransform(l_previousKeyFrame), FetchKeyFrameLocalTransform(l_nextKeyFrame), l_interpolationWeight);
}

// LocalSRTから行ベクトル方式のLocalMatrixを作成する
float4x4 CreateAnimationLocalMatrix(const SkeletalAnimationLocalTransform a_localTransform)
{
    const float4 l_rotation = NormalizeAnimationQuaternion(a_localTransform.rotation);
    
    const float l_xx = l_rotation.x * l_rotation.x;
    const float l_yy = l_rotation.y * l_rotation.y;
    const float l_zz = l_rotation.z * l_rotation.z;

    const float l_xy = l_rotation.x * l_rotation.y;
    const float l_xz = l_rotation.x * l_rotation.z;
    const float l_yz = l_rotation.y * l_rotation.z;
    
    const float l_xw = l_rotation.x * l_rotation.w;
    const float l_yw = l_rotation.y * l_rotation.w;
    const float l_zw = l_rotation.z * l_rotation.w;
    
    // SimpleMathと同じ行ベクトル方式でScale * Rotation * Translationを形成する
    return float4x4(a_localTransform.scale.x * (k_quaternionRotationMatrixBaseCoefficient   - k_quaternionRotationMatrixDoubleCoefficient  * (l_yy + l_zz)),
                    a_localTransform.scale.x * (k_quaternionRotationMatrixDoubleCoefficient * (l_xy                                        + l_zw)),
                    a_localTransform.scale.x * (k_quaternionRotationMatrixDoubleCoefficient * (l_xz                                        - l_yw)),
                    k_affineMatrixZeroElement,
                    a_localTransform.scale.y * (k_quaternionRotationMatrixDoubleCoefficient * (l_xy                                       - l_zw)),
                    a_localTransform.scale.y * (k_quaternionRotationMatrixBaseCoefficient   - k_quaternionRotationMatrixDoubleCoefficient * (l_xx + l_zz)),
                    a_localTransform.scale.y * (k_quaternionRotationMatrixDoubleCoefficient * (l_yz                                       + l_xw)),
                    k_affineMatrixZeroElement,
                    a_localTransform.scale.z * (k_quaternionRotationMatrixDoubleCoefficient * (l_xz                                       + l_yw)),
                    a_localTransform.scale.z * (k_quaternionRotationMatrixDoubleCoefficient * (l_yz                                       - l_xw)),
                    a_localTransform.scale.z * (k_quaternionRotationMatrixBaseCoefficient   - k_quaternionRotationMatrixDoubleCoefficient * (l_xx + l_yy)),
                    k_affineMatrixZeroElement,
                    a_localTransform.translation.x,
                    a_localTransform.translation.y,
                    a_localTransform.translation.z,
                    k_affineMatrixHomogeneousElement);
}