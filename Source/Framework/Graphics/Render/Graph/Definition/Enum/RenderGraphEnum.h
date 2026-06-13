#pragma once

namespace FWK::Enum
{
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
		PixelShaderResource,
		Present,
		Count,
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		RenderGraphResourceUsage,
		FWK_JSON_ENUM_VALUE(RenderGraphResourceUsage::Invalid),
		FWK_JSON_ENUM_VALUE(RenderGraphResourceUsage::RenderTarget),
		FWK_JSON_ENUM_VALUE(RenderGraphResourceUsage::PixelShaderResource),
		FWK_JSON_ENUM_VALUE(RenderGraphResourceUsage::Present),
		FWK_JSON_ENUM_VALUE(RenderGraphResourceUsage::Count)
	)
}