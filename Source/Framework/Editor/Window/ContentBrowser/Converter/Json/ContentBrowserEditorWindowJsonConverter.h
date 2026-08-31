#pragma once

namespace FWK::Editor
{
	class ContentBrowserEditorWindow;
}

namespace FWK::Converter
{
	class ContentBrowserEditorWindowJsonConverter
	{
	public:

		 ContentBrowserEditorWindowJsonConverter() = default;
		~ContentBrowserEditorWindowJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Editor::ContentBrowserEditorWindow& a_contentBrowserEditorWindow) const;

		nlohmann::json Serialize(const Editor::ContentBrowserEditorWindow& a_contentBrowserEditorWindow) const;

	private:

		void DeserializeAssetRegistryList(const nlohmann::json& a_rootJson, Editor::ContentBrowserEditorWindow& a_contentBrowserEditorWindow) const;

		nlohmann::json SerializeAssetRegistryList(const Editor::ContentBrowserEditorWindow& a_contentBrowserEditorWindow) const;

		std::string_view k_assetRegistryListJsonKey = "AssetRegistryList";
		std::string_view k_assetRegistryJsonKey     = "AssetRegistry";
	};
}