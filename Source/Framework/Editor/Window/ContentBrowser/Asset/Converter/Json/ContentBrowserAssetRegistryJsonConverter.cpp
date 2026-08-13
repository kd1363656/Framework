#include "ContentBrowserAssetRegistryJsonConverter.h"

void FWK::Converter::ContentBrowserAssetRegistryJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Editor::ContentBrowserAssetRegistry& a_contentBrowserAssetRegistry) const
{
	if (a_rootJson.is_null()) { return; }

	if (const auto& l_json = a_rootJson.value(k_assetFilePathToUUIDMapJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		DeserializeAssetFilePathToUUIDMap(l_json, a_contentBrowserAssetRegistry);
	}
}

nlohmann::json FWK::Converter::ContentBrowserAssetRegistryJsonConverter::Serialize(const Editor::ContentBrowserAssetRegistry& a_contentBrowserAssetRegistry) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_assetFilePathToUUIDMapJsonKey] = SerializeAssetFilePathToUUIDMap(a_contentBrowserAssetRegistry);
	
	return nlohmann::json();
}

void FWK::Converter::ContentBrowserAssetRegistryJsonConverter::DeserializeAssetFilePathToUUIDMap(const nlohmann::json& a_rootJson, Editor::ContentBrowserAssetRegistry& a_contentBrowserAssetRegistry) const
{
	if (!a_rootJson.is_null())             { return; }
	if (!Utility::IsJsonArray(a_rootJson)) { return; }

	for (const auto& l_json : a_rootJson)
	{
		if (l_json.is_null()) { continue;; }

		const auto& l_uuid     = Utility::DeserializeUUID(l_json, k_uuidJsonKey);
		const auto& l_filePath = l_json.value            (k_filePathJsonKey, std::filesystem::path{});

		// 読み込めないファイルパスがある場合はMapに追加しない
		if (!Utility::CanLoadFilePath(l_filePath)) { continue; }

		a_contentBrowserAssetRegistry.Add(l_uuid, l_filePath);
	}
}

nlohmann::json FWK::Converter::ContentBrowserAssetRegistryJsonConverter::SerializeAssetFilePathToUUIDMap(const Editor::ContentBrowserAssetRegistry& a_contentBrowserAssetRegistry) const
{
	auto l_rootJsonArray = nlohmann::json::array();

	const auto& l_assetFilePathToUUIDMap = a_contentBrowserAssetRegistry.GetREFAssetFilePathToUUIDMap();

	for (const auto& [l_filePath, l_uuid] : l_assetFilePathToUUIDMap)
	{
		// UUIDが無効化読み込めるファイルパスでなければシリアライズしない
		if (l_uuid.is_nil() ||
			!Utility::CanLoadFilePath(l_filePath))
		{
			continue; 
		}

		auto l_json = nlohmann::json{};

		l_json[k_filePathJsonKey] = l_filePath;
		l_json[k_uuidJsonKey]     = l_uuid;

		l_rootJsonArray.emplace_back(l_json);
	}

	return l_rootJsonArray;
}