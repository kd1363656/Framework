#pragma once

namespace FWK::Enum
{
	enum class RootSignatureType
	{
		Invalid,
		SpriteStandard,
		Count,
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		RootSignatureType,
		FWK_JSON_ENUM_VALUE(RootSignatureType::Invalid),
		FWK_JSON_ENUM_VALUE(RootSignatureType::SpriteStandard),
		FWK_JSON_ENUM_VALUE(RootSignatureType::Count),
	)

	enum class RootParameterType
	{
		Invalid,
		CBSpritePass,
		CBSpritePerObject,
		Count,
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		RootParameterType,
		FWK_JSON_ENUM_VALUE(RootParameterType::Invalid),
		FWK_JSON_ENUM_VALUE(RootParameterType::CBSpritePass),
		FWK_JSON_ENUM_VALUE(RootParameterType::CBSpritePerObject),
		FWK_JSON_ENUM_VALUE(RootParameterType::Count),
	)
}