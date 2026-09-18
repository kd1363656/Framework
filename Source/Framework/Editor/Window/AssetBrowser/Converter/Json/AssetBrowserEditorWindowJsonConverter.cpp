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

    // ペインスプリッターのデシリアライズ
    if (const auto& l_json = a_rootJson.value(k_editorWindowPaneSplitterJsonKey, nlohmann::json{});
        !l_json.is_null())
    {
        auto& l_paneSplitter = a_assetBrowserEditorWindow.GetMutableREFPaneSplitter();

        l_paneSplitter.Deserialize(l_json);
    }

    // 現在参照中のフォルダパスを復元
    // 従来FolderPaneJsonConverterにあったロジックをWindow側へ移動
    // 空or存在しない場合はAssetルートをデフォルトとする
    auto l_currentSelectFolderPath = a_rootJson.value(k_currentSelectFolderPathJsonKey, std::filesystem::path{});
    
    // パスが空、またはディスク上に存在しない場合はAssetルートをデフォルトとする
    // std::error_code版のexistsで例外を殴図に存在確認
    if (std::error_code l_errorCode = {};
        l_currentSelectFolderPath.empty() ||
        !std::filesystem::exists(l_currentSelectFolderPath, l_errorCode))
    {
        l_currentSelectFolderPath = Constant::k_assetRootFolderPath;
    }

    // Windowのpublicセッタ経由で現在参照中のフォルダをセット
    a_assetBrowserEditorWindow.SetCurrentSelectFolderPath(l_currentSelectFolderPath);
    
    // フォルダペインのデシリアライズ
    if (const auto& l_json = a_rootJson.value(k_folderPaneJsonKey, nlohmann::json{});
        !l_json.is_null())
    {
        auto& l_folderPane = a_assetBrowserEditorWindow.GetMutableREFFolderPane();

        l_folderPane.Deserialize(l_json);
    }

    // アセットペインのデシリアライズ
    if (const auto& l_json = a_rootJson.value(k_assetPaneJsonKey, nlohmann::json{});
        !l_json.is_null())
    {
        auto& l_assetPane = a_assetBrowserEditorWindow.GetMutableREFAssetPane();

        l_assetPane.Deserialize(l_json);
    }
}

nlohmann::json FWK::Converter::AssetBrowserEditorWindowJsonConverter::Serialize(const Editor::AssetBrowserEditorWindow& a_assetBrowserEditorWindow) const
{
          nlohmann::json l_rootJson              = {};
    const auto&          l_assetFilePathRegistry = a_assetBrowserEditorWindow.GetREFAssetFilePathRegistry();
    const auto&          l_paneSplitter          = a_assetBrowserEditorWindow.GetREFPaneSplitter         ();
    const auto&          l_folderPane            = a_assetBrowserEditorWindow.GetREFFolderPane           ();
    const auto&          l_assetPane             = a_assetBrowserEditorWindow.GetREFAssetPane            ();

    l_rootJson[k_assetFilePathRegistryJsonKey]    = l_assetFilePathRegistry.Serialize                      ();
    l_rootJson[k_editorWindowPaneSplitterJsonKey] = l_paneSplitter.Serialize                               ();
    l_rootJson[k_currentSelectFolderPathJsonKey] = a_assetBrowserEditorWindow.GetREFCurrentSelectFolderPath();
    l_rootJson[k_folderPaneJsonKey]               = l_folderPane.Serialize                                 ();
    l_rootJson[k_assetPaneJsonKey]                = l_assetPane.Serialize                                  ();

    return l_rootJson;
}