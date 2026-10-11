#pragma once

namespace FWK::Graphics
{
    class ModelMaterialFileCreator final
    {
    public:

         ModelMaterialFileCreator() = default;
        ~ModelMaterialFileCreator() = default;

        template <Concept::IsDerivedAssetRecordBaseConcept ModelRecordType>
        void CreateDefaultModelMaterialFileList(const std::filesystem::path& a_modelFilePath, const ModelRecordType& a_modelRecord)
        {}

    private:

        void CreateDefaultModelMaterialFile(const std::filesystem::path& a_modelFilePath, const std::wstring& a_subMeshName, const Struct::ModelMaterialAssetData& a_materialAssetData);

        AssetFilePath CreateTextureAssetFilePath(const std::filesystem::path& a_modelFilePath, const std::wstring& a_textureFilePath) const;

        static std::filesystem::path CreateDefaultModelMaterialFilePath(const std::filesystem::path& a_modelFilePath, const std::wstring& a_subMeshName);

        static std::wstring ReplaceInvalidFileNameCharacter(const std::wstring& a_name);

        static constexpr std::wstring_view k_invalidFileNameCharacterList = L"\\/:*?\"<>|";

        static constexpr wchar_t k_replaceFileNameCharacter = L'_';
    };
}