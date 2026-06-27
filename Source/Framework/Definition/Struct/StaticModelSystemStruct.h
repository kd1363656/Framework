#pragma once

namespace FWK::Struct
{
	struct StaticModelLoadResult final
	{
		std::weak_ptr<Graphics::StaticModelRecord> m_staticModelRecord = {};

		TypeAlias::StorageID m_storageID = Constant::k_invalidStorageID;
	};

	struct StaticModelBatchUploadRecord final
	{
		// StaticModelStorageへ登録するStaticModelRecord
		std::shared_ptr<Graphics::StaticModelRecord> m_staticModelRecord = nullptr;

		// StaticModelのBufferResourceへコピーするためのUploadCommand一覧
		std::vector<Struct::BufferUploadCommand> m_bufferUploadCommandList = {};
	};
}