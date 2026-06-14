#pragma once

namespace FWK::Enum
{
	enum class RootSignatureType
	{
		Invalid,
		SpriteScreen,
		FinalPresent,
		Count,
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		RootSignatureType,
		FWK_JSON_ENUM_VALUE(RootSignatureType::Invalid),
		FWK_JSON_ENUM_VALUE(RootSignatureType::SpriteScreen),
		FWK_JSON_ENUM_VALUE(RootSignatureType::FinalPresent),
		FWK_JSON_ENUM_VALUE(RootSignatureType::Count),
	)

	enum class RootParameterType
	{
		Invalid,
		CBSpritePass,
		CBSpritePerObject,
		CBFinalPresentPass,
		Count,
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		RootParameterType,
		FWK_JSON_ENUM_VALUE(RootParameterType::Invalid),
		FWK_JSON_ENUM_VALUE(RootParameterType::CBSpritePass),
		FWK_JSON_ENUM_VALUE(RootParameterType::CBSpritePerObject),
		FWK_JSON_ENUM_VALUE(RootParameterType::CBFinalPresentPass),
		FWK_JSON_ENUM_VALUE(RootParameterType::Count),
	)
}