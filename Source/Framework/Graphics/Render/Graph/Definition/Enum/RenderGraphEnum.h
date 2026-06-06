#pragma once

namespace FWK::Enum
{
	enum class RenderGraphPassType
	{
		None,
		BackBufferClear,
		BackBufferPresent,
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		RenderGraphPassType,
		FWK_JSON_ENUM_VALUE(RenderGraphPassType::None),
		FWK_JSON_ENUM_VALUE(RenderGraphPassType::BackBufferClear),
		FWK_JSON_ENUM_VALUE(RenderGraphPassType::BackBufferPresent)
	)

	enum class RenderGraphResourceType
	{
		None,
		BackBuffer,
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		RenderGraphResourceType,
		FWK_JSON_ENUM_VALUE(RenderGraphResourceType::None),
		FWK_JSON_ENUM_VALUE(RenderGraphResourceType::BackBuffer)
	)

	enum class RenderGraphResourceAccessType
	{
		None,
		Read,
		Write,
		ReadWrite,
		Present,
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		RenderGraphResourceAccessType,
		FWK_JSON_ENUM_VALUE(RenderGraphResourceAccessType::None),
		FWK_JSON_ENUM_VALUE(RenderGraphResourceAccessType::Read),
		FWK_JSON_ENUM_VALUE(RenderGraphResourceAccessType::Write),
		FWK_JSON_ENUM_VALUE(RenderGraphResourceAccessType::ReadWrite),
		FWK_JSON_ENUM_VALUE(RenderGraphResourceAccessType::ClearWrite),
		FWK_JSON_ENUM_VALUE(RenderGraphResourceAccessType::Present),
	)
}