#pragma once

namespace FWK::Enum
{
	enum class RenderGraphRenderTargetType
	{
		Invalid,
		None,
		SceneColor,
		FinalColor,
		Count,
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		RenderGraphRenderTargetType,
		FWK_JSON_ENUM_VALUE(RenderGraphRenderTargetType::Invalid),
		FWK_JSON_ENUM_VALUE(RenderGraphRenderTargetType::None),
		FWK_JSON_ENUM_VALUE(RenderGraphRenderTargetType::SceneColor),
		FWK_JSON_ENUM_VALUE(RenderGraphRenderTargetType::FinalColor),
		FWK_JSON_ENUM_VALUE(RenderGraphRenderTargetType::Count)
	)
}