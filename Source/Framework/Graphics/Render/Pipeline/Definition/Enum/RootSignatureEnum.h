#pragma once

namespace FWK::Enum
{
	enum class RootSignatureType : std::uint32_t
	{
		SpriteStandard,
		Count
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		RootSignatureType,
		FWK_JSON_ENUM_VALUE(RootSignatureType::SpriteStandard),
		FWK_JSON_ENUM_VALUE(RootSignatureType::Count)
	)

	enum class RootParameterSlot : std::uint32_t
	{
		PassCBV,
		Count
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		RootParameterSlot,
		FWK_JSON_ENUM_VALUE(RootParameterSlot::PassCBV),
		FWK_JSON_ENUM_VALUE(RootParameterSlot::Count)
	)
}