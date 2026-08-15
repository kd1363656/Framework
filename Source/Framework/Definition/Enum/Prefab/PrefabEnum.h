#pragma once

namespace FWK::Enum
{
	enum class PrefabType
	{
		None,
		GameObject,
		Scene,
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		PrefabType,
		FWK_JSON_ENUM_VALUE(PrefabType::None),
		FWK_JSON_ENUM_VALUE(PrefabType::GameObject),
		FWK_JSON_ENUM_VALUE(PrefabType::Scene),
	);
}