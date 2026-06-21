#pragma once

namespace FWK::Enum
{
	enum class RenderGraphDepthStencilType
	{
		Invalid,
		None,
		SceneDepth,
		Count,
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		RenderGraphDepthStencilType,
		FWK_JSON_ENUM_VALUE(RenderGraphDepthStencilType::Invalid),
		FWK_JSON_ENUM_VALUE(RenderGraphDepthStencilType::None),
		FWK_JSON_ENUM_VALUE(RenderGraphDepthStencilType::SceneDepth),
		FWK_JSON_ENUM_VALUE(RenderGraphDepthStencilType::Count)
	)
}