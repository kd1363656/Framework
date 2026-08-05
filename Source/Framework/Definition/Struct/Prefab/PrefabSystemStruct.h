#pragma once

namespace FWK::Struct
{
	struct PrefabData final
	{
		Prefab m_prefab = {};

		PrefabInstanceNUMAllocator m_prefabInstanceNUMAllocator = {};
	};
}