#pragma once

namespace FWK::Enum
{
    enum class PipelineStateType
    {
        Invalid,
        SpriteScreen,
        StaticModelCascadeShadow,
        StaticModelUnLit,
        StaticModelLit,
        SkeletalAnimationVertexSkinning,
        SkeletalAnimationMeshletBoundsUpdate,
        SkeletalAnimationModelCascadeShadow,
        SkeletalAnimationModelUnLit,
        SkeletalAnimationModelLit,
        FinalColor,
        PhysicsDebug,
        FinalPresent,
        ModelGPUDrivenCulling,
        HierarchicalDepthBuild,
        SkeletalAnimationModelGPUSkinnedLit,
        SkeletalAnimationModelGPUSkinnedCascadeShadow,
        Count,
    };

    FWK_JSON_SERIALIZE_ENUM
    (
        PipelineStateType,
        FWK_JSON_ENUM_VALUE(PipelineStateType::Invalid),
        FWK_JSON_ENUM_VALUE(PipelineStateType::SpriteScreen),
        FWK_JSON_ENUM_VALUE(PipelineStateType::StaticModelUnLit),
        FWK_JSON_ENUM_VALUE(PipelineStateType::StaticModelLit),
        FWK_JSON_ENUM_VALUE(PipelineStateType::StaticModelCascadeShadow),
        FWK_JSON_ENUM_VALUE(PipelineStateType::SkeletalAnimationMeshletBoundsUpdate),
        FWK_JSON_ENUM_VALUE(PipelineStateType::SkeletalAnimationVertexSkinning),
        FWK_JSON_ENUM_VALUE(PipelineStateType::SkeletalAnimationModelCascadeShadow),
        FWK_JSON_ENUM_VALUE(PipelineStateType::SkeletalAnimationModelUnLit),
        FWK_JSON_ENUM_VALUE(PipelineStateType::SkeletalAnimationModelLit),
        FWK_JSON_ENUM_VALUE(PipelineStateType::FinalColor),
        FWK_JSON_ENUM_VALUE(PipelineStateType::PhysicsDebug),
        FWK_JSON_ENUM_VALUE(PipelineStateType::FinalPresent),
        FWK_JSON_ENUM_VALUE(PipelineStateType::ModelGPUDrivenCulling),
        FWK_JSON_ENUM_VALUE(PipelineStateType::HierarchicalDepthBuild),
        FWK_JSON_ENUM_VALUE(PipelineStateType::SkeletalAnimationModelGPUSkinnedLit),
        FWK_JSON_ENUM_VALUE(PipelineStateType::SkeletalAnimationModelGPUSkinnedCascadeShadow),
        FWK_JSON_ENUM_VALUE(PipelineStateType::Count)
    )
}