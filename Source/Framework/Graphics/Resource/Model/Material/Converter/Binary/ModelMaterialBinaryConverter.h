#pragma once

namespace FWK
{
    class AssetFilePath;
}

namespace FWK::Graphics
{
    class ModelMaterialBase;
}

namespace FWK::Converter
{
    class ModelMaterialBinaryConverter final : public BinaryConverterBase
    {
    private:

        struct ModelMaterialBinaryHeader final
        {
            std::uint64_t m_fileSize     = k_emptyAssetFileSize;
            std::uint64_t m_typeNameSize = k_emptyAssetFileSize;
            std::uint16_t m_version      = k_modelMaterialAssetVersion;
            std::uint16_t m_assetTypeID  = k_modelMaterialAssetTypeID;
        };

    public:

         ModelMaterialBinaryConverter()          = default;
        ~ModelMaterialBinaryConverter() override = default;

        bool Load(const std::filesystem::path& a_filePath, Graphics::ModelMaterialBase& a_material);

        bool Save(const std::filesystem::path& a_filePath, const Graphics::ModelMaterialBase& a_material);

        bool TryReadTypeName(const std::filesystem::path& a_filePath, std::string& a_typeName);

        bool TryReadAssetFilePath(AssetFilePath& a_assetFilePath, std::uint64_t& a_memoryReadOffset) const;

        template <typename Type>
        bool TryReadMaterialValue(Type& a_value, std::uint64_t& a_memoryReadOffset) const
        {
            return false;
        }

        void WriteAssetFilePath(const AssetFilePath& a_assetFilePath, std::uint64_t& a_memoryWriteOffset) const;

        template <typename Type>
        void WriteMaterialValue(const Type& a_value, std::uint64_t& a_memoryWriteOffset) const
        {}

        std::uint64_t CalculateAssetFilePathBinaryDataSize() const;

        template <typename Type>
        std::uint64_t CalculateMaterialValueBinaryDataSize() const
        {
            return k_emptyAssetFileSize;
        }

    private:

        bool TryReadModelMaterialBinaryHeader(ModelMaterialBinaryHeader& a_header, std::uint64_t& a_memoryReadOffset) const;

        static constexpr std::uint16_t k_modelMaterialAssetTypeID = 0x4D54U;

        static constexpr std::uint16_t k_modelMaterialAssetVersion = 1U;
    };
}