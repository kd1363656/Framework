#pragma once

namespace FWK::Editor
{
    class AssetBrowserEditorWindowDirectoryDeleteChange final : public AssetBrowserEditorWindowDirectoryChangeBase
    {
    public:

         AssetBrowserEditorWindowDirectoryDeleteChange()          = default;
        ~AssetBrowserEditorWindowDirectoryDeleteChange() override = default;

        void Apply(AssetFilePathRegistry& a_assetFilePathRegistry, SceneManager& a_sceneManager) override;

    private:

        void ApplyFileDelete(const std::filesystem::path& a_deleteFilePath, AssetFilePathRegistry& a_assetFilePathRegistry, SceneManager& a_sceneManager) const;

        void ApplyPrefabDelete(const std::filesystem::path& a_deleteFilePath,
                               const boost::uuids::uuid&    a_prefabUUID,
                               const SceneManager&          a_sceneManager,
                                     AssetFilePathRegistry& a_assetFilePathRegistry) const;

        void ApplySceneDelete(const std::filesystem::path&  a_deleteFilePath,
                               const boost::uuids::uuid&    a_sceneUUID,
                                     AssetFilePathRegistry& a_assetFilePathRegistry,
                                     SceneManager&          a_sceneManager) const;


        void ApplyDirectoryDelete(const std::filesystem::path& a_deleteFilePath, AssetFilePathRegistry& a_assetFilePathRegistry, SceneManager& a_sceneManager) const;
    };
}