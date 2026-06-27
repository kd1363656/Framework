#pragma once

namespace FWK::Enum
{
	enum class DefaultTextureType : std::uint32_t
	{
		BaseColor,
		Normal,
		Metallic,
		Roughness,
		Count,
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		DefaultTextureType,
		FWK_JSON_ENUM_VALUE(DefaultTextureType::BaseColor),
		FWK_JSON_ENUM_VALUE(DefaultTextureType::Normal),
		FWK_JSON_ENUM_VALUE(DefaultTextureType::Metallic),
		FWK_JSON_ENUM_VALUE(DefaultTextureType::Roughness),
		FWK_JSON_ENUM_VALUE(DefaultTextureType::Count)
	)

	enum class DefaultTextureColorChannel : std::uint32_t
	{
		R,
		G,
		B,
		A,
		Count
	};
}