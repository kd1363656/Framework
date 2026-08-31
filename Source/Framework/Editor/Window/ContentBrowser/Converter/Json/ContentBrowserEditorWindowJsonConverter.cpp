#include "ContentBrowserEditorWindowJsonConverter.h"

void FWK::Converter::ContentBrowserEditorWindowJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Editor::ContentBrowserEditorWindow& a_contentBrowserEditorWindow) const
{
	if (a_rootJson.is_null()) { return; }

	if (const auto& l_json = a_rootJson.value(k_assetRegistryListJsonKey, nlohmann::json{});
		!l_json.is_null() &&
		Utility::IsJsonArray(l_json))
	{
		DeserializeAssetRegistryList(l_json, a_contentBrowserEditorWindow);
	}
}

nlohmann::json FWK::Converter::ContentBrowserEditorWindowJsonConverter::Serialize(const Editor::ContentBrowserEditorWindow& a_contentBrowserEditorWindow) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_assetRegistryListJsonKey] = SerializeAssetRegistryList(a_contentBrowserEditorWindow);

	return l_rootJson;
}

void FWK::Converter::ContentBrowserEditorWindowJsonConverter::DeserializeAssetRegistryList(const nlohmann::json& a_rootJson, Editor::ContentBrowserEditorWindow& a_contentBrowserEditorWindow) const
{
	if (a_rootJson.is_null() ||
		!Utility::IsJsonArray(a_rootJson))
	{
		return; 
	}

	auto& l_assetRegistryList = a_contentBrowserEditorWindow.GetMutableREFAssetRegistryList();

	if (l_assetRegistryList.empty() ||
		a_rootJson.empty()) 

	{
		return;
	}

	for (std::size_t l_i = 0ULL; l_i < l_assetRegistryList.size(); ++l_i)
	{
		if (a_rootJson.size() <= l_i ||
			l_assetRegistryList.size() <= l_i)
		{
			return; 
		}

		const auto& l_json = a_rootJson[l_i];

		if (l_json.is_null()) { continue; }

		auto& l_assetRegistry = l_assetRegistryList[l_i];

		// AssetRegistryの復元
		if (l_json.contains(k_assetRegistryJsonKey))
		{
			l_assetRegistry.Deserialize(l_json[k_assetRegistryJsonKey]);
		}
	}
}
nlohmann::json FWK::Converter::ContentBrowserEditorWindowJsonConverter::SerializeAssetRegistryList(const Editor::ContentBrowserEditorWindow& a_contentBrowserEditorWindow) const
{
	      auto  l_rootJsonArray     = nlohmann::json::array                               ();
	const auto& l_assetRegistryList = a_contentBrowserEditorWindow.GetREFAssetRegistryList();

	for (const auto& l_assetRegistry : l_assetRegistryList)
	{
		auto l_json = nlohmann::json{};

		l_json[k_assetRegistryJsonKey] = l_assetRegistry.Serialize();

		l_rootJsonArray.emplace_back(l_json);
	}

	return l_rootJsonArray;
}