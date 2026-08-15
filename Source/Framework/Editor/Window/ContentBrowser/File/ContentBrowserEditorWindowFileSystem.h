#pragma once

namespace FWK::Editor
{
	class ContentBrowserEditorWindowFileSystem
	{
	public:

		 ContentBrowserEditorWindowFileSystem() = default;
		~ContentBrowserEditorWindowFileSystem() = default;

		std::filesystem::path CreateFolder              (const std::filesystem::path&     a_parentFolderPath, const std::string&           a_folderName) const;
		std::filesystem::path CreatePrefabFromGameObject(const std::weak_ptr<GameObject>& a_gameObject,       const std::filesystem::path& a_parentFolderPath, ContentBrowserEditorWindowAssetRegistry& a_assetRegistry) const;
		
		bool DeleteFolder    (const std::filesystem::path& a_folderPath,     ContentBrowserEditorWindowAssetRegistry& a_assetRegistry) const;
		bool DeletePrefabFile(const std::filesystem::path& a_prefabFilePath, ContentBrowserEditorWindowAssetRegistry& a_assetRegistry) const;

		bool HasChildFolder(const std::filesystem::path& a_folderPath) const;


	private:

		static constexpr std::uintmax_t k_notRemovedEntryCount = 0U;
	};
}