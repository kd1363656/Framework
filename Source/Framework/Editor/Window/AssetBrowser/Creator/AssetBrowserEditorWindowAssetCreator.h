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

        template <Enum::AssetFilePathRegistryType RegistryType>
        Struct::AssetBrowserEditorWindowAssetCreationResult CreateRegisteredAssetFile(const std::filesystem::path& a_parentFolderPath, const std::string_view& a_defaultName, AssetFilePathRegistry& a_assetFilePathRegistry) const
        {
            const auto& l_filePath = ResolveDefaultFilePath(a_parentFolderPath, Constant::k_lowerJsonExtension, a_defaultName);
            const auto& l_uuid     = RegisterNewAssetFile  (l_filePath, RegistryType, a_assetFilePathRegistry);

            if (l_uuid.is_nil()) { return {}; }

            if (!WriteNewAssetFile<RegistryType>(l_filePath, l_uuid))
            {
                CancelNewAssetFile(l_filePath, a_assetFilePathRegistry);

                return {};
            }

            return FetchVALSuccessCreationResult(l_filePath);
        }

        template <Enum::AssetFilePathRegistryType RegistryType>
        bool WriteNewAssetFile(const std::filesystem::path& a_filePath, const boost::uuids::uuid& a_uuid) const
        {
            static_assert(RegistryType == Enum::AssetFilePathRegistryType::Prefab ||
                          RegistryType == Enum::AssetFilePathRegistryType::Scene,
                          "書き込み方が決まっていないアセットの種類です");

            if constexpr (RegistryType == Enum::AssetFilePathRegistryType::Prefab)
            {
                return WriteNewPrefabFile(a_filePath, a_uuid);
            }
            else
            {
                return WriteNewSceneFile(a_filePath);
            }
        }

        bool WriteNewPrefabFile(const std::filesystem::path& a_filePath, const boost::uuids::uuid& a_prefabUUID) const;
        bool WriteNewSceneFile (const std::filesystem::path& a_filePath)                                         const;

        static boost::uuids::uuid RegisterNewAssetFile(const std::filesystem::path& a_filePath, const Enum::AssetFilePathRegistryType a_registryType, AssetFilePathRegistry& a_assetFilePathRegistry);

        static void CancelNewAssetFile(const std::filesystem::path& a_filePath, AssetFilePathRegistry& a_assetFilePathRegistry);

        static Struct::AssetBrowserEditorWindowAssetCreationResult FetchVALSuccessCreationResult(const std::filesystem::path& a_createdFilePath);

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
