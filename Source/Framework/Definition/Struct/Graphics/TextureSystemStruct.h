#pragma once

namespace FWK::Struct
{
	struct TextureLoadResult final
	{
		std::weak_ptr<Graphics::TextureRecord> m_textureRecord = {};

		TypeAlias::StorageID m_storageID = Graphics::AssetRecordBase::k_invalidStorageID;

		bool m_isLoadSuccess = false;
	};
}