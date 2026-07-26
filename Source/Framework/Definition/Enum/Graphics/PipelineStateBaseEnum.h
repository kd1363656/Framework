#pragma once

namespace FWK::Enum
{
	enum class PipelineStateType
	{
		Invalid,
		SpriteScreen,
		StaticModelUnLit,
		StaticModelLit,
		StaticModelCascadeShadow,
		SkeletalAnimationVertexSkinning,
		SkeletalAnimationMeshletBoundsUpdate,
		SkeletalAnimationModelUnLit,
		SkeletalAnimationModelLit,
		FinalColor,
		PhysicsDebug,
		FinalPresent,
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
		FWK_JSON_ENUM_VALUE(PipelineStateType::SkeletalAnimationModelUnLit),
		FWK_JSON_ENUM_VALUE(PipelineStateType::SkeletalAnimationModelLit),
		FWK_JSON_ENUM_VALUE(PipelineStateType::FinalColor),
		FWK_JSON_ENUM_VALUE(PipelineStateType::PhysicsDebug),
		FWK_JSON_ENUM_VALUE(PipelineStateType::FinalPresent),
		FWK_JSON_ENUM_VALUE(PipelineStateType::Count)
	)
}