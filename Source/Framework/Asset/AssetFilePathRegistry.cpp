#include "AssetFilePathRegistry.h"

void FWK::AssetFilePathRegistry::INIT()
{
	m_assetFilePathToUUIDMap.clear ();
	m_uuidToAssetFilePathData.clear();

	m_jsonConverter = {};
}

void FWK::AssetFilePathRegistry::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

nlohmann::json FWK::AssetFilePathRegistry::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

bool FWK::AssetFilePathRegistry::Add(const std::filesystem::path& a_assetFilePath, const boost::uuids::uuid& a_assetUUID, Enum::AssetFilePathRegistryType a_assetFilePathRegisterType)
{
	FWK_ASSERT_RETURN_VALUE_IF(a_assetFilePath.empty(), "AssetFilePathが空のため、AssetFilePathRegistryへの登録に失敗しました。",  false);

	// 無効なUUIDなら登録しない
	if (a_assetUUID.is_nil())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "AssetFilePathRegistryに追加する予定のUUIDが無効値を示しており、AssetFilePathRegistryへの登録に失敗しました。");

		return false;
	}

	if (a_assetFilePathRegisterType == Enum::AssetFilePathRegistryType::Invalid)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "AssetFilePathRegistryに追加する予定のAssetFilePathRegistryTypeが無効値を示しており、AssetFilePathRegistryへの登録に失敗しました。");

		return false;
	}

	// UUIDが既に登録されているなら登録しない
	if (m_uuidToAssetFilePathData.contains(a_assetUUID))
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "AssetFilePathRegistryに追加する予定のUUIDが既に登録されており、AssetFilePathRegistryへの登録に失敗しました。");

		return false;
	}

	if (!m_assetFilePathToUUIDMap.try_emplace(a_assetFilePath, a_assetUUID).second)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "AssetFilePathRegistryへの登録に失敗しました。");

		return false;
	}

	if (Struct::AssetFilePathData l_assetFilePathData = { a_assetFilePath, a_assetFilePathRegisterType };
		!m_uuidToAssetFilePathData.try_emplace(a_assetUUID, l_assetFilePathData).second)
	{
		// 2爪のMap登録に失敗したら一つ目のマップから削除
		m_assetFilePathToUUIDMap.erase(a_assetFilePath);

		FWK_ADD_LOG(Constant::k_debugWarningColor, "AssetFilePathRegistryへの登録に失敗しました。");

		return false;
	}

	return true;
}

bool FWK::AssetFilePathRegistry::ReplaceFilePath(const std::filesystem::path& a_oldAssetFilePath, const std::filesystem::path& a_newAssetFilePath)
{
	if (a_oldAssetFilePath.empty() ||
		a_newAssetFilePath.empty())
	{
		return false;
	}

	if (a_oldAssetFilePath == a_newAssetFilePath) { return true; }

	// 変更後Pathが既に別AssetのKeyとして存在する場合は、
	// 同じPathへ2つのAssetを登録できないため変更しない
	if (m_assetFilePathToUUIDMap.contains(a_newAssetFilePath))
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor,
			        "AssetFilePathRegistryで変更後FilePathが既に登録されています。\nOldFilePath : {}\nNewFilePath : {}",
			        a_oldAssetFilePath.string(),
			        a_newAssetFilePath.string());

		return false;
	}

	// extract()はunordered_map内部から要素をNodeHandleとして切り出す
	// 通常unordered_mapのKeyは変更できないが
	// extract()したNodeHandleのKey()は変更可能
	auto l_filePathNode = m_assetFilePathToUUIDMap.extract(a_oldAssetFilePath);

	if (l_filePathNode.empty()) { return false; }

	// UUIDは変更せずKeyだけ変更する
	l_filePathNode.key() = a_newAssetFilePath;

	// UUID -> AssetFilePathData側も同じUUIDを使って検索する
	const auto& l_assetUUID            = l_filePathNode.mapped         ();
	const auto  l_assetFilePathDataITR = m_uuidToAssetFilePathData.find(l_assetUUID);

	if (l_assetFilePathDataITR == m_uuidToAssetFilePathData.end()) 
	{
		// 内部Registryの対応関係がおかしいので
		// Keyを書き換える前のPathへ戻してRegistryへ復元する
		l_filePathNode.key() = a_oldAssetFilePath;

		m_assetFilePathToUUIDMap.insert(std::move(l_filePathNode));

		FWK_ADD_LOG(Constant::k_debugWarningColor, "AssetFilePathRegistry内部でUUIDとAssetFilePathDataの対応関係が不正です。");

		return false;
	}

	// UUID -> AssetFilePathData側のPathもNewPathへ変更する
	auto& l_assetFilePathData = l_assetFilePathDataITR->second;

	l_assetFilePathData.m_assetFilePath = a_newAssetFilePath;

	// Keyを書き換えたNodeをunordered_mapへ戻す
	m_assetFilePathToUUIDMap.insert(std::move(l_filePathNode));

	return true;
}

bool FWK::AssetFilePathRegistry::Erase(const std::filesystem::path& a_assetFilePath)
{
	FWK_ASSERT_RETURN_VALUE_IF(a_assetFilePath.empty(), "AssetFilePathが空のため、AssetFilePathRegistryから削除できませんでした。", false);

	const auto& l_itr = m_assetFilePathToUUIDMap.find(a_assetFilePath);

	FWK_ASSERT_RETURN_VALUE_IF(l_itr == m_assetFilePathToUUIDMap.end(), "指定されたAssetFilePathがAssetFilePathRegistryへ登録されていません。", false);

	// UUID重複確認用Setから削除予定のマップのUUIDを削除
	auto l_uuid = l_itr->second;

	m_uuidToAssetFilePathData.erase(l_uuid);
	m_assetFilePathToUUIDMap.erase (l_itr);

	return true;
}

const boost::uuids::uuid* FWK::AssetFilePathRegistry::FindPTRAssetUUID(const std::filesystem::path& a_assetFilePath) const
{
	if (a_assetFilePath.empty()) { return nullptr; }

	const auto& l_itr = m_assetFilePathToUUIDMap.find(a_assetFilePath);

	if (l_itr == m_assetFilePathToUUIDMap.end()) { return nullptr; }

	return &l_itr->second;
}
const FWK::Struct::AssetFilePathData* FWK::AssetFilePathRegistry::FindPTRAssetFilePathData(const boost::uuids::uuid& a_uuid) const
{
	if (a_uuid.is_nil()) { return nullptr; }

	const auto& l_itr = m_uuidToAssetFilePathData.find(a_uuid);

	if (l_itr == m_uuidToAssetFilePathData.end()) { return nullptr; }

	return &l_itr->second;
}

bool FWK::AssetFilePathRegistry::ContainsAssetUUID(const boost::uuids::uuid& a_assetUUID) const
{
	return m_uuidToAssetFilePathData.contains(a_assetUUID);
}