#include "ContentBrowserEditorWindowJsonConverter.h"

void FWK::Converter::ContentBrowserEditorWindowJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Editor::ContentBrowserEditorWindow& a_contentBrowserEditorWindow) const
{
	if (a_rootJson.is_null()) { return; }

	auto& l_assetRegistry = a_contentBrowserEditorWindow.GetMutableREFAssetRegistry();

	if (const auto& l_json = a_rootJson.value(k_assetRegistryJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		l_assetRegistry.Deserialize(l_json);
	}
}

nlohmann::json FWK::Converter::ContentBrowserEditorWindowJsonConverter::Serialize(const Editor::ContentBrowserEditorWindow& a_contentBrowserEditorWindow) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_assetRegistry = a_contentBrowserEditorWindow.GetREFAssetRegistry();

	l_rootJson[k_assetRegistryJsonKey] = l_assetRegistry.Serialize();

	return l_rootJson;
}