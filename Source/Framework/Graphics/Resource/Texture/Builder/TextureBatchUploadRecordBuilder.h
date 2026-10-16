#pragma once

namespace FWK::Graphics
{
    class TextureBatchUploadRecordBuilder final
    {
    public:

        struct TextureUploadRecord final
        {
             TextureUploadRecord() = default;
            ~TextureUploadRecord() = default;

            TextureUploadRecord(const TextureUploadRecord&)           = delete;
            TextureUploadRecord(      TextureUploadRecord&&) noexcept = default;

            TextureUploadRecord& operator=(const TextureUploadRecord&)           = delete;
            TextureUploadRecord& operator=(      TextureUploadRecord&&) noexcept = default;

            // 各サブリソースの配置情報をまとめたリスト
            std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> m_layoutList = {};

            // DEFAULTヒープ上のテクスチャリソースへコピーするための中間バッファ
            Graphics::UploadBuffer m_uploadBuffer = {};
        };

        struct TextureBatchUploadRecord final
        {
             TextureBatchUploadRecord() = default;
            ~TextureBatchUploadRecord() = default;

            TextureBatchUploadRecord(const TextureBatchUploadRecord&)           = delete;
            TextureBatchUploadRecord(      TextureBatchUploadRecord&&) noexcept = default;

            TextureBatchUploadRecord& operator=(const TextureBatchUploadRecord&)           = delete;
            TextureBatchUploadRecord& operator=(      TextureBatchUploadRecord&&) noexcept = default;

            std::shared_ptr<Graphics::TextureRecord> m_textureRecord = nullptr;

            TextureUploadRecord m_textureUploadRecord = {};
        };

    public:

         TextureBatchUploadRecordBuilder() = default;
        ~TextureBatchUploadRecordBuilder() = default;

        bool CreateTextureBatchUploadRecord(const DirectX::ScratchImage&              a_scratchImage,
                                            const DirectX::TexMetadata&               a_texMetadata,
                                            const std::wstring&                       a_filePath,
                                            const Device&                             a_device,
                                            const GPUMemoryAllocator&                 a_gpuMemoryAllocator,
                                            const TypeAlias::StorageID                a_storageID,
                                                  TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool,
                                                  TextureBatchUploadRecord&           a_textureBatchUploadRecord) const;

    private:

        bool CreateTextureResource(const DirectX::TexMetadata& a_texMetadata, const GPUMemoryAllocator& a_gpuMemoryAllocator, Graphics::TextureRecord& a_textureRecord) const;

        bool CreateTextureUploadRecord(const DirectX::ScratchImage& a_scratchImage, const Device& a_device, TextureBatchUploadRecord& a_textureBatchUploadRecord) const;

        bool CreateTextureSRV(const DirectX::TexMetadata&               a_texMetadata,
                              const Device&                             a_device,
                                    Graphics::TextureRecord&            a_textureRecord,
                                    TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool) const;

        static constexpr FLOAT k_resourceMINLODClamp = 0.0F;

        static constexpr UINT64 k_uploadBufferBeginOffset         = 0ULL;
        static constexpr UINT64 k_initialRequiredUploadBufferSize = 0ULL;

        static constexpr std::size_t k_texture2DArrayMINArraySize = 2ULL;

        static constexpr UINT k_mostDetailedMIP = 0U;
        static constexpr UINT k_planeSlice      = 0U;
        static constexpr UINT k_firstArraySlice = 0U;
    };
}