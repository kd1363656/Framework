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
        std::shared_ptr<Graphics::StaticModelRecord> m_staticModelRecord = nullptr;

        std::vector<Graphics::StaticStructuredBuffer::BufferUploadCommand> m_bufferUploadCommandList = {};
    };
}