#pragma once

namespace FWK::Enum
{
	enum class DefaultTextureType : std::uint32_t
	{
		BaseColor,
		Normal,
		Count,
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		DefaultTextureType,
		FWK_JSON_ENUM_VALUE(DefaultTextureType::BaseColor),
		FWK_JSON_ENUM_VALUE(DefaultTextureType::Normal),
		FWK_JSON_ENUM_VALUE(DefaultTextureType::Count)
	)
}