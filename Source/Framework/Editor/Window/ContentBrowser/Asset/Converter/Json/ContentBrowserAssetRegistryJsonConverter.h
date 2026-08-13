#pragma once

namespace FWK::Editor
{
	class ContentBrowserAssetRegistry;
}

namespace FWK::Converter
{
	class ContentBrowserAssetRegistryJsonConverter
	{
	public:

		 ContentBrowserAssetRegistryJsonConverter() = default;
		~ContentBrowserAssetRegistryJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Editor::ContentBrowserAssetRegistry& a_contentBrowserAssetRegistry) const;

		nlohmann::json Serialize(const Editor::ContentBrowserAssetRegistry& a_contentBrowserAssetRegistry) const;

	private:

		void DeserializeAssetFilePathToUUIDMap(const nlohmann::json& a_rootJson, Editor::ContentBrowserAssetRegistry& a_contentBrowserAssetRegistry) const;
		
		nlohmann::json SerializeAssetFilePathToUUIDMap(const Editor::ContentBrowserAssetRegistry& a_contentBrowserAssetRegistry) const;
		
		std::string_view k_assetFilePathToUUIDMapJsonKey = "AssetFilePathToUUIDMap";
		std::string_view k_filePathJsonKey               = "FilePath";
		std::string_view k_uuidJsonKey                   = "UUID";
	};
}