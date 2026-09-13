#pragma once

namespace FWK::Editor
{
    class AssetBrowserEditorWindowAssetCreator final
    {
    public:

         AssetBrowserEditorWindowAssetCreator() = default;
        ~AssetBrowserEditorWindowAssetCreator() = default;

        Struct::AssetBrowserEditorWindowAssetCreationResult CreateFolder(const std::filesystem::path& a_parentFolderPath)                                                 const;
        Struct::AssetBrowserEditorWindowAssetCreationResult CreatePrefab(const std::filesystem::path& a_parentFolderPath, AssetFilePathRegistry& a_assetFilePathRegistry) const;
        Struct::AssetBrowserEditorWindowAssetCreationResult CreateScene (const std::filesystem::path& a_parentFolderPath, AssetFilePathRegistry& a_assetFilePathRegistry) const;

    private:

        static std::filesystem::path ResolveDefaultFilePath(const std::filesystem::path& a_parentFolderPath, const std::filesystem::path& a_extension, const std::string_view& a_defaultName);

        static constexpr std::string_view k_defaultFolderName = "NewFolder";
        static constexpr std::string_view k_defaultPrefabName = "NewPrefab";
        static constexpr std::string_view k_defaultSceneName  = "NewScene";
    };
}
