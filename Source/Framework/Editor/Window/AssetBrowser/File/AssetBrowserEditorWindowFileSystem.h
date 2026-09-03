#pragma once

namespace FWK::Editor
{
	class AssetBrowserEditorWindowFileSystem
	{
	public:

		 AssetBrowserEditorWindowFileSystem() = default;
		~AssetBrowserEditorWindowFileSystem() = default;

		std::filesystem::path CreateFolder              (const std::filesystem::path&     a_parentFolderPath, const std::filesystem::path& a_folderName) const;
		std::filesystem::path CreatePrefabFromGameObject(const std::weak_ptr<GameObject>& a_gameObject,       const std::filesystem::path& a_parentFolderPath, AssetFilePathRegistry& a_assetFilePathRegistry) const;
		std::filesystem::path CreateSceneFromScene      (const std::weak_ptr<Scene>&      a_scene,            const std::filesystem::path& a_parentFolderPath, AssetFilePathRegistry& a_assetFilePathRegistry) const;
		
		bool DeleteFolder     (const std::filesystem::path& a_folderPath,     AssetFilePathRegistry& a_assetFilePathRegistry) const;
		bool DeletePrefabFile (const std::filesystem::path& a_prefabFilePath, AssetFilePathRegistry& a_assetFilePathRegistry) const;
		bool DeleteSceneFile  (const std::filesystem::path& a_sceneFilePath,  AssetFilePathRegistry& a_assetFilePathRegistry) const;
		bool DeleteRegularFile(const std::filesystem::path& a_filePath)                                                       const;

		bool HasChildFolder(const std::filesystem::path& a_folderPath) const;

	private:

		static constexpr std::uintmax_t k_notRemovedEntryCount = 0U;
	};
}