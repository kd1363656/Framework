#pragma once

namespace FWK::Enum
{
	enum class RenderGraphResourceType
	{
		Invalid,
		BackBuffer,
		SceneColor,
		Count,
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		RenderGraphResourceType,
		FWK_JSON_ENUM_VALUE(RenderGraphResourceType::Invalid),
		FWK_JSON_ENUM_VALUE(RenderGraphResourceType::BackBuffer),
		FWK_JSON_ENUM_VALUE(RenderGraphResourceType::SceneColor),
		FWK_JSON_ENUM_VALUE(RenderGraphResourceType::Count)
	)
}