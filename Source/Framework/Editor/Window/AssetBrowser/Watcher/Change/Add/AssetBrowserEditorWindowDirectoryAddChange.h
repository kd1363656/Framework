#pragma once

namespace FWK::Editor
{
    class AssetBrowserEditorWindowDirectoryAddChange final : public AssetBrowserEditorWindowDirectoryChangeBase
    {
    public:

         AssetBrowserEditorWindowDirectoryAddChange()          = default;
        ~AssetBrowserEditorWindowDirectoryAddChange() override = default;

        void Apply(AssetFilePathRegistry& a_assetFilePathRegistry, SceneManager& a_sceneManager) override;

    private:

        void ApplyPrefabAdd(const std::filesystem::path& a_filePath, const SceneManager&       a_sceneManager,const boost::uuids::uuid&    a_prefabUUID);
        void ApplySceneAdd (const std::filesystem::path& a_filePath, const boost::uuids::uuid& a_sceneUUID,   const AssetFilePathRegistry& a_assetFilePathRegistry, SceneManager& a_sceneManager);
    };
}