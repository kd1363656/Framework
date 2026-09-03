#pragma once

namespace FWK::Enum
{
	enum class AssetFilePathRegistryType
	{
		Invalid,
		Prefab,
		Scene,
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		AssetFilePathRegistryType,
		FWK_JSON_ENUM_VALUE(AssetFilePathRegistryType::Invalid),
		FWK_JSON_ENUM_VALUE(AssetFilePathRegistryType::Prefab),
		FWK_JSON_ENUM_VALUE(AssetFilePathRegistryType::Scene),
	)
}