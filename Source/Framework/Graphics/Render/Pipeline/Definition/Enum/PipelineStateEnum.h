#pragma once

namespace FWK::Enum
{
	enum class PipelineStateType
	{
		Invalid,
		SpriteScreen,
		Count,
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		PipelineStateType,
		FWK_JSON_ENUM_VALUE(PipelineStateType::Invalid),
		FWK_JSON_ENUM_VALUE(PipelineStateType::SpriteScreen),
		FWK_JSON_ENUM_VALUE(PipelineStateType::Count)
	)
}