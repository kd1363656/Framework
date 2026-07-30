#pragma once

namespace FWK::Struct
{
	struct PrefabData final
	{
		Prefab m_prefab = {};

		Utility::StorageIDAllocator m_prefabNumberAllocator = {};
	};
}