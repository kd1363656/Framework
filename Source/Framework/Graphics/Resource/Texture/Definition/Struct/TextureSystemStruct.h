#pragma once

namespace FWK::Graphics
{
	class TextureRecord;
}

namespace FWK::Struct
{
	struct TextureLoadResult final
	{
		std::weak_ptr<Graphics::TextureRecord> m_textureRecord = {};

		TypeAlias::StorageID m_storageID = Constant::k_invalidStorageID;
	};
}