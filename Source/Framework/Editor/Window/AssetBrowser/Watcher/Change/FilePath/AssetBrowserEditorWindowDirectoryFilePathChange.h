#pragma once

namespace FWK::Editor
{
	class AssetBrowserEditorWindowDirectoryFilePathChange final : public AssetBrowserEditorWindowDirectoryChangeBase
	{
	public:

		 AssetBrowserEditorWindowDirectoryFilePathChange()          = default;
		~AssetBrowserEditorWindowDirectoryFilePathChange() override = default;

		void Apply(AssetFilePathRegistry& a_assetBrowserAssetFilePathRegistry, SceneManager& a_sceneManager) override;

		void SetNewFilePath(const std::filesystem::path& a_set) { m_newFilePath = a_set; }

	private:

		void ApplyFilePathChange(const std::filesystem::path& a_oldFilePath, 
			                     const std::filesystem::path& a_newFilePath,
			                           AssetFilePathRegistry& a_assetBrowserAssetFilePathRegistry,
			                           SceneManager&          a_sceneManager) const;

		void ApplyPrefabFilePathChange(const std::filesystem::path& a_oldFilePath, 
			                           const std::filesystem::path& a_newFilePath,
			                           const boost::uuids::uuid&    a_prefabUUID,
			                                 AssetFilePathRegistry& a_assetBrowserAssetFilePathRegistry,
			                                 SceneManager&          a_sceneManager) const;

		void ApplySceneFilePathChange(const std::filesystem::path& a_oldFilePath, 
			                          const std::filesystem::path& a_newFilePath,
			                          const boost::uuids::uuid&    a_sceneUUID,
			                                AssetFilePathRegistry& a_assetBrowserAssetFilePathRegistry,
			                                SceneManager&          a_sceneManager) const;

		void ApplyDirectoryFilePathChange(const std::filesystem::path& a_oldFilePath,
			                              const std::filesystem::path& a_newFilePath, 
			                                    AssetFilePathRegistry& a_assetBrowserAssetFilePathRegistry,
			                                    SceneManager&          a_sceneManager) const;

		std::filesystem::path m_newFilePath = {};
	};
}