#pragma once

namespace FWK::Editor
{
	class AssetBrowserEditorWindowDirectoryAddChange final : public AssetBrowserEditorWindowDirectoryChangeBase
	{
	public:

		 AssetBrowserEditorWindowDirectoryAddChange()          = default;
		~AssetBrowserEditorWindowDirectoryAddChange() override = default;

		void Apply(AssetFilePathRegistry& a_assetBrowserAssetFilePathRegistry, SceneManager& a_sceneManager) override;

	private:

		void ApplyPrefabAdd(const std::filesystem::path& a_filePath, const boost::uuids::uuid& a_prefabUUID, SceneManager& a_sceneManager) const;
		void ApplySceneAdd (const std::filesystem::path& a_filePath, const boost::uuids::uuid& a_sceneUUID,  SceneManager& a_sceneManager) const;
	};
}