#pragma once

namespace FWK::Editor
{
	class ContentBrowserEditorWindowAssetRegistry;
}

namespace FWK::Converter
{
	class ContentBrowserEditorWindowAssetRegistryJsonConverter
	{
	public:

		 ContentBrowserEditorWindowAssetRegistryJsonConverter() = default;
		~ContentBrowserEditorWindowAssetRegistryJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Editor::ContentBrowserEditorWindowAssetRegistry& a_contentBrowserEditorWindowAssetRegistry) const;

		nlohmann::json Serialize(const Editor::ContentBrowserEditorWindowAssetRegistry& a_contentBrowserEditorWindowAssetRegistry) const;

	private:

		void DeserializeAssetFilePathToUUIDMap(const nlohmann::json& a_rootJson, Editor::ContentBrowserEditorWindowAssetRegistry& a_contentBrowserEditorWindowAssetRegistry) const;
		
		nlohmann::json SerializeAssetFilePathToUUIDMap(const Editor::ContentBrowserEditorWindowAssetRegistry& a_contentBrowserEditorWindowAssetRegistry) const;
		
		static constexpr std::string_view k_assetFilePathToUUIDMapJsonKey = "AssetFilePathToUUIDMap";
		static constexpr std::string_view k_filePathJsonKey               = "FilePath";
		static constexpr std::string_view k_assetUUIDJsonKey              = "AssetUUID";
	};
}