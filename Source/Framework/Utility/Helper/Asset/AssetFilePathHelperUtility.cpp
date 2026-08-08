#include "AssetFilePathHelperUtility.h"

void FWK::Utility::AssetFilePathHelper::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

nlohmann::json FWK::Utility::AssetFilePathHelper::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

bool FWK::Utility::AssetFilePathHelper::ReceiveFilePathDragDrop()
{
    std::filesystem::path l_filePath = {};

    // 直前に描画されたImGuiItemへFilePathがDropされた場合だけ、
    // DropされたFilePathを受け取る
    if (!Utility::FilePathDragDropTarget(Constant::k_assetFilePathDragAndDropPayloadLabel, l_filePath)) { return false; }

    // FilePathの最終的な妥当性確認はApplyAssetFilePathへ集約する
    return ApplyAssetFilePath(l_filePath);
}

bool FWK::Utility::AssetFilePathHelper::ApplyAssetFilePath(const std::filesystem::path& a_set)
{
    // 許可され邸内拡張子だった場合、フィルパスを更新しない
    if (a_set.empty())
    {
        m_allowedFileExtension.empty();

        return;
    }

    // 読み込み可能拡張子が設定されていなければAssetを受け付けない
    if (m_allowedFileExtension.empty()) { return; }

    // Fileが存在すること
    // RegularFileであること
    // 許可買う調子と一致することを確認する
    if (!Utility::CanLoadFilePath(a_set, m_allowedFileExtension)) { return; }

    m_assetFilePath = a_set;
}