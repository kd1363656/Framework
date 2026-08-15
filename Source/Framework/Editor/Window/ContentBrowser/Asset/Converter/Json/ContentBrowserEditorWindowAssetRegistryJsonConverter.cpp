#include "ContentBrowserEditorWindowAssetRegistryJsonConverter.h"

void FWK::Converter::ContentBrowserEditorWindowAssetRegistryJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Editor::ContentBrowserEditorWindowAssetRegistry& a_contentBrowserEditorWindowAssetRegistry) const
{
	if (a_rootJson.is_null()) { return; }

	if (const auto& l_json = a_rootJson.value(k_assetFilePathToUUIDMapJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		DeserializeAssetFilePathToUUIDMap(l_json, a_contentBrowserEditorWindowAssetRegistry);
	}
}
nlohmann::json FWK::Converter::ContentBrowserEditorWindowAssetRegistryJsonConverter::Serialize(const Editor::ContentBrowserEditorWindowAssetRegistry& a_contentBrowserEditorWindowAssetRegistry) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_assetFilePathToUUIDMapJsonKey] = SerializeAssetFilePathToUUIDMap(a_contentBrowserEditorWindowAssetRegistry);

	return l_rootJson;
}

void FWK::Converter::ContentBrowserEditorWindowAssetRegistryJsonConverter::DeserializeAssetFilePathToUUIDMap(const nlohmann::json& a_rootJson, Editor::ContentBrowserEditorWindowAssetRegistry& a_contentBrowserEditorWindowAssetRegistry) const
{
	if (a_rootJson.is_null())              { return; }
	if (!Utility::IsJsonArray(a_rootJson)) { return; }

	for (const auto& l_json : a_rootJson)
	{
		if (l_json.is_null()) { continue; }

		const auto& l_uuid     = Utility::DeserializeUUID(l_json, k_assetUUIDJsonKey);
		const auto& l_filePath = l_json.value            (k_filePathJsonKey, std::filesystem::path{});

		// 読み込めないファイルパスがある場合はMapに追加しない
		if (!Utility::CanLoadFilePath(l_filePath)) { continue; }

		a_contentBrowserEditorWindowAssetRegistry.Add(l_uuid, l_filePath);
	}
}
nlohmann::json FWK::Converter::ContentBrowserEditorWindowAssetRegistryJsonConverter::SerializeAssetFilePathToUUIDMap(const Editor::ContentBrowserEditorWindowAssetRegistry& a_contentBrowserEditorWindowAssetRegistry) const
{
	auto l_rootJsonArray = nlohmann::json::array();

	const auto& l_assetFilePathToUUIDMap = a_contentBrowserEditorWindowAssetRegistry.GetREFAssetFilePathToUUIDMap();

	for (const auto& [l_filePath, l_assetUUID] : l_assetFilePathToUUIDMap)
	{
		// UUIDが無効化読み込めるファイルパスでなければシリアライズしない
		if (l_assetUUID.is_nil() ||
			!Utility::CanLoadFilePath(l_filePath))
		{
			continue; 
		}

		auto l_json = nlohmann::json{};

		l_json[k_filePathJsonKey]  = l_filePath;
		Utility::UpdateJson(l_json, Utility::SerializeUUID(l_assetUUID, k_assetUUIDJsonKey));

		l_rootJsonArray.emplace_back(l_json);
	}

	return l_rootJsonArray;
}