#pragma once

namespace FWK::Enum
{
	enum class RenderGraphPassType
	{
		Invalid,
		SpriteStandard,
		Count,
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		RenderGraphPassType,
		FWK_JSON_ENUM_VALUE(RenderGraphPassType::Invalid),
		FWK_JSON_ENUM_VALUE(RenderGraphPassType::SpriteStandard),
		FWK_JSON_ENUM_VALUE(RenderGraphPassType::Count)
	)

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

	enum class RenderGraphAccessType
	{
		Invalid,
		Read,
		Write,
		Count,
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		RenderGraphAccessType,
		FWK_JSON_ENUM_VALUE(RenderGraphAccessType::Invalid),
		FWK_JSON_ENUM_VALUE(RenderGraphAccessType::Read),
		FWK_JSON_ENUM_VALUE(RenderGraphAccessType::Write),
		FWK_JSON_ENUM_VALUE(RenderGraphAccessType::Count)
	)

	enum class RenderGraphResourceUsage
	{
		Invalid,
		RenderTarget,
		Present,
		Count,
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		RenderGraphResourceUsage,
		FWK_JSON_ENUM_VALUE(RenderGraphResourceUsage::Invalid),
		FWK_JSON_ENUM_VALUE(RenderGraphResourceUsage::RenderTarget),
		FWK_JSON_ENUM_VALUE(RenderGraphResourceUsage::Present),
		FWK_JSON_ENUM_VALUE(RenderGraphResourceUsage::Count)
	)
}