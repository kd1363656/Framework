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

        std::vector<Struct::AssetBrowserEditorWindowAssetCreationResult> CreatePrefabFromGameObjectDrop(const std::weak_ptr<GameObject>& a_droppedGameObject, 
                                                                                                        const std::filesystem::path&     a_parentFolderPath,
                                                                                                              Scene&                     a_scene,
                                                                                                              AssetFilePathRegistry&     a_assetFilePathRegistry) const;
 

        void RenamePrefab(const std::filesystem::path& a_oldFilePath, const std::filesystem::path& a_newFilePath) const;
        void RenameScene (const std::filesystem::path& a_oldFilePath, const std::filesystem::path& a_newFilePath) const;

    private:

        static std::filesystem::path ResolveDefaultFilePath(const std::filesystem::path&     a_parentFolderPath, const std::filesystem::path& a_extension, const std::string_view& a_defaultName);

        static std::string FetchVALPrefabFileName(const std::weak_ptr<GameObject>& a_gameObject);

        Struct::AssetBrowserEditorWindowAssetCreationResult CreatePrefabFromGameObject(const std::weak_ptr<GameObject>& a_gameObject, 
                                                                                       const std::filesystem::path&     a_parentFolderPath,
                                                                                             Scene&                     a_scene,
                                                                                             AssetFilePathRegistry&     a_assetFilePathRegistry) const;
 
        std::vector<std::shared_ptr<GameObject>> CollectPrefabTargetGameObjectList(const std::weak_ptr<GameObject>& a_droppedGameObject) const;
 

        static constexpr std::string_view k_defaultFolderName = "NewFolder";
        static constexpr std::string_view k_defaultPrefabName = "NewPrefab";
        static constexpr std::string_view k_defaultSceneName  = "NewScene";

        static constexpr std::string_view k_invalidFileNameCharacters = "\\/:*?\"<>|";

        static constexpr char k_underScoreChar = '_';
        static constexpr char k_dotChar        = '.';
        static constexpr char k_spaceChar      = ' ';
    };
}
