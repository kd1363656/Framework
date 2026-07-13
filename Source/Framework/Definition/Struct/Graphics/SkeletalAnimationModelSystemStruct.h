#pragma once

namespace FWK::Struct
{
	struct SkeletalAnimationModelLoadResult final
	{
		std::weak_ptr<Graphics::SkeletalAnimationModelRecord> m_skeletalAnimationModelRecord = {};

		TypeAlias::StorageID m_storageID = Graphics::AssetRecordBase::k_invalidStorageID;
	};

	struct SkeletalAnimationModelBatchUploadRecord final
	{
		std::shared_ptr<Graphics::SkeletalAnimationModelRecord> m_skeletalAnimationModelRecord = nullptr;

		std::vector<Graphics::StaticStructuredBuffer::BufferUploadCommand> m_bufferUploadCommandList = {};
	};
}