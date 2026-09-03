#ifndef MODEL_SKELETAL_ANIMATION_MODEL_HLSLI
#define MODEL_SKELETAL_ANIMATION_MODEL_HLSLI

struct SkeletalAnimationModelVertex
{
    float3 position;
    float3 normal;
    float4 tangent;
    float2 uv;
    
    float4 boneWeight;
    
    uint bonePaletteIndexZero;
    uint bonePaletteIndexOne;
    uint bonePaletteIndexTwo;
    uint bonePaletteIndexThree;
};

struct SkeletalAnimationModelBonePaletteElement
{
    row_major float4x4 inverseBindPoseMatrix;
    
    uint boneIndex;
};

struct SkeletalAnimationBoneMatrixElement
{
    row_major float4x4 globalBoneMatrix;
};

struct SkeletalAnimationSkinnedVertex
{
    float3 position;
    float3 normal;
    float4 tangent;
    float2 uv;
};

#endif // MODEL_SKELETAL_ANIMATION_MODEL_HLSLI