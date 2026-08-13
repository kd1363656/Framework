#include "ContentBrowserAssetRegistry.h"

void FWK::Editor::ContentBrowserAssetRegistry::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

nlohmann::json FWK::Editor::ContentBrowserAssetRegistry::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

bool FWK::Editor::ContentBrowserAssetRegistry::Add(const boost::uuids::uuid& a_assetUUID, const std::filesystem::path& a_assetFilePath)
{
	FWK_ASSERT_RETURN_VALUE_IF(a_assetFilePath.empty() || 
		                       !std::filesystem::exists(a_assetFilePath),
		                       "AssetFilePathが空のため、ContentBrowserAssetRegistryへの登録に失敗しました。", 
		                       false);

	// 無効なUUIDなら登録しない
	if (a_assetUUID.is_nil())
	{
		FWK_ADD_LOG("ContentBrowserAssetRegistryに追加する予定のUUIDが無効値を示しており、ContentBrowserAssetRegistryへの登録に失敗しました。");

		return false;
	}

	// UUIDが既に登録されているなら登録しない
	if (m_assetUUIDSet.contains(a_assetUUID))
	{
		FWK_ADD_LOG("ContentBrowserAssetRegistryに追加する予定のUUIDが既に登録されており、ContentBrowserAssetRegistryへの登録に失敗しました。");

		return false;
	}

	m_assetFilePathToUUIDMap.try_emplace(a_assetFilePath, a_assetUUID);
	m_assetUUIDSet.emplace              (a_assetUUID);

	return true;
}

bool FWK::Editor::ContentBrowserAssetRegistry::Erace(const std::filesystem::path& a_assetFilePath)
{
	FWK_ASSERT_RETURN_VALUE_IF(a_assetFilePath.empty(), "AssetFilePathが空のため、ContentBrowserAssetRegistryから削除できませんでした。", false);

	const auto& l_itr = m_assetFilePathToUUIDMap.find(a_assetFilePath);

	FWK_ASSERT_RETURN_VALUE_IF(l_itr == m_assetFilePathToUUIDMap.end(), "指定されたAssetFilePathがContentBrowserAssetRegistryへ登録されていません。", false);

	// UUID重複確認用Setから削除予定のマップのUUIDを削除
	m_assetUUIDSet.erase          (l_itr->second);
	m_assetFilePathToUUIDMap.erase(l_itr);

	return true;
}

boost::uuids::uuid FWK::Editor::ContentBrowserAssetRegistry::FindPTRAssetUUID(const std::filesystem::path& a_assetFilePath) const
{
	if (a_assetFilePath.empty()) { return; }

	const auto& l_itr = m_assetFilePathToUUIDMap.find(a_assetFilePath);

	if (l_itr == m_assetFilePathToUUIDMap.end()) { return; }

	return l_itr->second;
}