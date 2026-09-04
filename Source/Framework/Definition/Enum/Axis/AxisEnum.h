#pragma once

namespace FWK::Enum
{
	enum class Axis : std::uint32_t
	{
		Invalid = 0U,
		X       = 1U << 0U,
		Y       = 2U << 0U,
		Z       = 3U << 0U
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		Axis,
		FWK_JSON_ENUM_VALUE(Axis::Invalid),
		FWK_JSON_ENUM_VALUE(Axis::X),
		FWK_JSON_ENUM_VALUE(Axis::Y),
		FWK_JSON_ENUM_VALUE(Axis::Z),
	)
}

FWK_REGISTER_STRING_VALUE_BIDIRECTIONAL_REGISTRY(FWK::Enum::Axis, FWK::Enum::Axis::X)
FWK_REGISTER_STRING_VALUE_BIDIRECTIONAL_REGISTRY(FWK::Enum::Axis, FWK::Enum::Axis::Y)
FWK_REGISTER_STRING_VALUE_BIDIRECTIONAL_REGISTRY(FWK::Enum::Axis, FWK::Enum::Axis::Z)