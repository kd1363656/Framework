#include "AssetBrowserEditorWindowFolderPaneJsonConverter.h"

void FWK::Converter::AssetBrowserEditorWindowFolderPaneJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Editor::AssetBrowserEditorWindowFolderPane& a_assetBrowserEditorWindowFolderPane) const
{
    if (a_rootJson.is_null()) { return; }

    // 前のプロジェクトで参照していたフォルダパスを復元
    auto l_currentFolderPath = a_rootJson.value(k_currentFolderPathJsonKey, std::filesystem::path{});

    // 指定したパスが空、またはディスク上に存在しない場合
    // Assetルートフォルダを現在参照中のフォルダとして扱う
    std::error_code l_errorCode = {};

    if (l_currentFolderPath.empty() ||
        !std::filesystem::exists(l_currentFolderPath, l_errorCode))
    {
        l_currentFolderPath = Constant::k_assetRootFolderPath;
    }

    FWK_ASSERT_RETURN_IF(l_errorCode, "現在参照中ののフォルダパスが存在するかどうかの確認に失敗しました");

    // FolderPaneのpublicセッター経由で現在参照中のフォルダをセット
    a_assetBrowserEditorWindowFolderPane.SetCurrentFolderPath(l_currentFolderPath);

    // フォルダツリーの開閉状態を復元
    if (const auto& l_json = a_rootJson.value(k_folderOpenStateMapJsonKey, nlohmann::json{});
        !l_json.is_null() &&
        Utility::IsJsonArray(l_json))
    {
        DeserializeOpenStateMap(l_json, a_assetBrowserEditorWindowFolderPane);
    }
}

nlohmann::json FWK::Converter::AssetBrowserEditorWindowFolderPaneJsonConverter::Serialize(const Editor::AssetBrowserEditorWindowFolderPane& a_assetBrowserEditorWindowFolderPane) const
{
          nlohmann::json l_rootJson        = {};
    const auto&          l_currentFilePath = a_assetBrowserEditorWindowFolderPane.GetREfCurrentFolderPath();

    l_rootJson[k_currentFolderPathJsonKey]  = l_currentFilePath;
    l_rootJson[k_folderOpenStateMapJsonKey] = SerializeOpenStateMap(a_assetBrowserEditorWindowFolderPane);

    return l_rootJson;
}

void FWK::Converter::AssetBrowserEditorWindowFolderPaneJsonConverter::DeserializeOpenStateMap(const nlohmann::json& a_rootJson, Editor::AssetBrowserEditorWindowFolderPane& a_assetBrowserEditorWindowFolderPane) const
{
    if (a_rootJson.is_null() ||
        !Utility::IsJsonArray(a_rootJson)) 
    {
        return;
    }

    for (const auto& l_json : a_rootJson)
    {
        const auto& l_folderPath = l_json.value(k_folderOpenStateFilePathJsonKey, std::filesystem::path{});
        const auto  l_isOpen     = l_json.value(k_folderOpenStateIsOpenJsonKey,   k_initialIsFolderOpen);

        // ファイルが存在しなければMapには追加しない
        if (l_folderPath.empty() ||
            !std::filesystem::exists(l_folderPath))
        {
            continue;
        }

        a_assetBrowserEditorWindowFolderPane.AddFolderOpenState(l_folderPath, l_isOpen);
    }
}

nlohmann::json FWK::Converter::AssetBrowserEditorWindowFolderPaneJsonConverter::SerializeOpenStateMap(const Editor::AssetBrowserEditorWindowFolderPane& a_assetBrowserEditorWindowFolderPane) const
{
           auto l_rootJsonArray      = nlohmann::json::array                                        ();
    const auto& l_folderOpenStateMap = a_assetBrowserEditorWindowFolderPane.GetREFFolderOpenStateMap();

    for (const auto& [l_key, l_value] : l_folderOpenStateMap)
    {
        nlohmann::json l_json = {};

        l_json[k_folderOpenStateFilePathJsonKey] = l_key;
        l_json[k_folderOpenStateIsOpenJsonKey]   = l_value;

        l_rootJsonArray.emplace_back(l_json);
    }

    return l_rootJsonArray;
}
