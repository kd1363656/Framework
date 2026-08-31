#pragma once

namespace FWK::Enum
{
	enum class ContentBrowserAssetType : std::uint8_t
	{
		Prefab,
		Scene,
		Count,
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		ContentBrowserAssetType,
		FWK_JSON_ENUM_VALUE(ContentBrowserAssetType::Prefab),
		FWK_JSON_ENUM_VALUE(ContentBrowserAssetType::Scene),
		FWK_JSON_ENUM_VALUE(ContentBrowserAssetType::Count),
	)
}