#pragma once

namespace FWK::Editor
{
	class AssetBrowserEditorWindow;
}

namespace FWK::Converter
{
	class AssetBrowserEditorWindowJsonConverter
	{
	public:

		 AssetBrowserEditorWindowJsonConverter() = default;
		~AssetBrowserEditorWindowJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Editor::AssetBrowserEditorWindow& a_assetBrowserEditorWindow) const;

		nlohmann::json Serialize(const Editor::AssetBrowserEditorWindow& a_assetBrowserEditorWindow) const;
	};
}