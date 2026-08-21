#include "AssetFilePathHelper.h"

void FWK::Utility::AssetFilePathHelper::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

nlohmann::json FWK::Utility::AssetFilePathHelper::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

void FWK::Utility::AssetFilePathHelper::EditInspector()
{
    m_inspector.EditInspector(*this);
}

bool FWK::Utility::AssetFilePathHelper::ApplyAssetFilePath(const std::filesystem::path& a_set)
{
    // 許可されていない拡張子だった場合、フィルパスを更新しない
    if (a_set.empty()) { return true; }

    // 読み込み可能拡張子が設定されていなければAssetを受け付けない
    if (m_allowedFileExtension.empty()) { return false; }

    // Fileが存在すること
    // RegularFileであること
    // 許可されている拡張子と一致することを確認する
    if (!Utility::CanLoadFilePath(a_set, m_allowedFileExtension)) { return false; }

    m_assetFilePath = a_set;

    return true;
}