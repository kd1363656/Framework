#pragma once

namespace FWK::Enum
{
	enum class RenderGraphShadowMapType 
	{
		Invalid,
		None,
		Cascade,
		Count
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		RenderGraphShadowMapType,
		FWK_JSON_ENUM_VALUE(RenderGraphShadowMapType::Invalid),
		FWK_JSON_ENUM_VALUE(RenderGraphShadowMapType::None),
		FWK_JSON_ENUM_VALUE(RenderGraphShadowMapType::Cascade),
		FWK_JSON_ENUM_VALUE(RenderGraphShadowMapType::Count),
	);
}