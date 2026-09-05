#include "AssetBrowserEditorWindowJsonConverter.h"

void FWK::Converter::AssetBrowserEditorWindowJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Editor::AssetBrowserEditorWindow& a_assetBrowserEditorWindow) const
{
	if (a_rootJson.is_null()) { return; }

	// アセットファイルパスレジストリーのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_assetFilePathRegistryJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_assetFilePathRegistry = a_assetBrowserEditorWindow.GetMutableREFAssetFilePathRegistry();

		l_assetFilePathRegistry.Deserialize(l_json);
	}
}

nlohmann::json FWK::Converter::AssetBrowserEditorWindowJsonConverter::Serialize(const Editor::AssetBrowserEditorWindow& a_assetBrowserEditorWindow) const
{
	      nlohmann::json l_rootJson              = {};
	const auto&          l_assetFilePathRegistry = a_assetBrowserEditorWindow.GetREFAssetFilePathRegistry();

	l_rootJson[k_assetFilePathRegistryJsonKey] = l_assetFilePathRegistry.Serialize();

	return l_rootJson;
}