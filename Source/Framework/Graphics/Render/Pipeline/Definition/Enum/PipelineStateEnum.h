#pragma once

namespace FWK::Enum
{
	enum class PipelineStateType : std::uint32_t
	{
		SpriteStandard,
		Count
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		PipelineStateType,
		FWK_JSON_ENUM_VALUE(PipelineStateType::SpriteStandard),
		FWK_JSON_ENUM_VALUE(PipelineStateType::Count)
	)
}