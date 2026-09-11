#pragma once

namespace FWK::Enum
{
	enum class AxisBitShiftFlag : std::uint32_t
	{
		Invalid = 0U,
		X       = 1U << 0U,
		Y       = 2U << 0U,
		Z       = 3U << 0U
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		AxisBitShiftFlag,
		FWK_JSON_ENUM_VALUE(AxisBitShiftFlag::Invalid),
		FWK_JSON_ENUM_VALUE(AxisBitShiftFlag::X),
		FWK_JSON_ENUM_VALUE(AxisBitShiftFlag::Y),
		FWK_JSON_ENUM_VALUE(AxisBitShiftFlag::Z),
	)
}

FWK_REGISTER_STRING_VALUE_BIDIRECTIONAL_REGISTRY(FWK::Enum::AxisBitShiftFlag, FWK::Enum::AxisBitShiftFlag::X)
FWK_REGISTER_STRING_VALUE_BIDIRECTIONAL_REGISTRY(FWK::Enum::AxisBitShiftFlag, FWK::Enum::AxisBitShiftFlag::Y)
FWK_REGISTER_STRING_VALUE_BIDIRECTIONAL_REGISTRY(FWK::Enum::AxisBitShiftFlag, FWK::Enum::AxisBitShiftFlag::Z)