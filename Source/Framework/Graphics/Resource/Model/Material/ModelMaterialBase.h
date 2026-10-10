#pragma once

namespace FWK::Struct
{
    struct ModelRenderTableINFO;
}

namespace FWK::Graphics
{
    class ModelMaterialBase
    {
    public:

                 ModelMaterialBase();
        virtual ~ModelMaterialBase();

        ModelMaterialBase(const ModelMaterialBase&)  = delete;
        ModelMaterialBase(      ModelMaterialBase&&) = delete;

        ModelMaterialBase& operator=(const ModelMaterialBase&)  = delete;
        ModelMaterialBase& operator=(      ModelMaterialBase&&) = delete;

        bool Load(const std::filesystem::path& a_filePath);

        bool Save(const std::filesystem::path& a_filePath);

        virtual bool TryReadBinaryData(const Converter::ModelMaterialBinaryConverter& a_binaryConverter, std::uint64_t& a_memoryReadOffset);
        virtual void WriteBinaryData  (const Converter::ModelMaterialBinaryConverter& a_binaryConverter, std::uint64_t& a_memoryWriteOffset) const;

        virtual std::uint64_t CalculateBinaryDataSize(const Converter::ModelMaterialBinaryConverter& a_binaryConverter) const;

        virtual void LoadRuntimeTextures();

        bool CreateGPUData();

        void ApplyGPUData() const;

        void SetBaseColorTexture(const AssetFilePath& a_set) { m_baseColorTexture = a_set; }

        void SetBaseColor(const TypeAlias::Math::Color& a_set) { m_baseColor = a_set; }

        virtual const Struct::ModelRenderTableINFO& FetchREFTableINFO() const = 0;

        const auto& GetREFBaseColorTexture() const { return m_baseColorTexture; }

        const auto& GetREFBaseColorRuntimeTexture() const { return m_baseColorRuntimeTexture; }

        const auto& GetREFBaseColor() const { return m_baseColor; }

        auto GetVALTableElementIndex() const { return m_tableElementIndex; }

    protected:

        virtual void WriteGPUData(GPUElementTable& a_table, const std::uint32_t a_tableElementIndex) const = 0;

        void LoadRuntimeTexture(const AssetFilePath&              a_assetFilePath,
                                const Enum::TextureLoadColorSpace a_textureLoadColorSpace,
                                const Enum::DefaultTextureType    a_defaultTextureType,
                                      Texture&                    a_runtimeTexture) const;

        TypeAlias::DescriptorIndex FetchVALTextureSRVDescriptorIndex(const Texture& a_texture) const;

    private:

        void ReleaseGPUData();

        std::weak_ptr<GPUElementTable> m_table;

        AssetFilePath m_baseColorTexture;
        Texture       m_baseColorRuntimeTexture;

        Converter::ModelMaterialBinaryConverter m_binaryConverter;

        TypeAlias::Math::Color m_baseColor;

        std::uint32_t m_tableElementIndex;

        FWK_DEFINE_TYPE_INFO_ROOT(ModelMaterialBase)
    };
}