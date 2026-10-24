#pragma once

namespace FWK::Struct
{
    struct MaterialLoadResult final
    {
        std::weak_ptr<Graphics::MaterialBase> m_material = {};

        // デフォルトマテリアルは参照数で管理しないため、StorageIDは無効値のままになる
        TypeAlias::StorageID m_storageID = Constant::k_invalidStorageID;
    };

    struct MaterialBatchUploadRecord final
    {
        // MaterialStorageへ登録するMaterial
        std::shared_ptr<Graphics::MaterialBase> m_material = nullptr;

        // MaterialのGPUData用BufferResourceへコピーするためのUploadCommand一覧
        std::vector<Graphics::StaticStructuredBuffer::BufferUploadCommand> m_bufferUploadCommandList = {};
    };
}