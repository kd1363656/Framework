#include "AssetBrowserEditorWindowJsonConverter.h"

void FWK::Converter::AssetBrowserEditorWindowJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Editor::AssetBrowserEditorWindow& a_assetBrowserEditorWindow) const
{
	if (a_rootJson.is_null()) { return; }
}

nlohmann::json FWK::Converter::AssetBrowserEditorWindowJsonConverter::Serialize(const Editor::AssetBrowserEditorWindow& a_assetBrowserEditorWindow) const
{
	nlohmann::json l_rootJson = {};
	
	return l_rootJson;
}