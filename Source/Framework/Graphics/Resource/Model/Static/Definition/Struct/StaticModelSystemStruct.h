#pragma once

namespace FWK::Struct
{
	struct StaticModelResult final
	{
		std::weak_ptr<Graphics::StaticModelRecord> m_staticModelRecord = {};

		TypeAlias::StorageID m_storageID = Constant::k_invalidStorageID;
	};
}