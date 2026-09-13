#include "AssetBrowserEditorWindowPopupDrawer.h"

void FWK::Editor::AssetBrowserEditorWindowPopupDrawer::Draw(const std::vector<std::filesystem::path>&          a_selectedFilePathList,
                                                            const std::filesystem::path&                       a_targetFilePath, 
                                                            const std::filesystem::path&                       a_parentFolderPath, 
                                                            const Enum::AssetBrowserPopupContextType           a_contextType,
                                                                  AssetBrowserEditorWindowAssetCreator&        a_assetCreator, 
                                                                  AssetBrowserEditorWindowClipboard&           a_clipboard, 
                                                                  AssetFilePathRegistry&                       a_filePathRegistry, 
                                                                  Struct::AssetBrowserEditorWindowRenameState& a_renameState) const
{
    // AssetPaneの空白右クリックかどうか
    const bool l_isAssetPaneEmpty = (a_contextType == Enum::AssetBrowserPopupContextType::AssetPane_OnEmpty);

    // 新規フォルダ作成が可能か
    const bool l_canCreateFolder = (a_contextType == Enum::AssetBrowserPopupContextType::FolderPane_OnFolder) ||
                                   (a_contextType == Enum::AssetBrowserPopupContextType::AssetPane_OnFolder)  ||
                                   (a_contextType == Enum::AssetBrowserPopupContextType::AssetPane_OnEmpty);

    // 新規プレハブ・新規シーンが作成可能か
    const bool l_canCreatePrefab = l_isAssetPaneEmpty;
    const bool l_canCreateScene  = l_isAssetPaneEmpty;

    // 名前変更可能か
    // OnFolder(両Pane)・OnFile(AssetPane)で可能
    // OnEmptyでは対象がないため不可
    const bool l_canRename = (a_contextType == Enum::AssetBrowserPopupContextType::FolderPane_OnFolder) ||
                             (a_contextType == Enum::AssetBrowserPopupContextType::AssetPane_OnFolder)  ||
                             (a_contextType == Enum::AssetBrowserPopupContextType::AssetPane_OnFile);

    // 選択中のファイルがあるか(コピー / 切り取り / 複製 / 削除の判定に使用)
    const bool l_hasSelection = !a_selectedFilePathList.empty();

    // クリップボードが空でないか(貼り付けの判定に使用)
    // Clipboard::IsEmpty()はconst参照で調べる
    const bool l_canPaste = !a_clipboard.IsEmpty();


}

void FWK::Editor::AssetBrowserEditorWindowPopupDrawer::StartRename(const std::filesystem::path& a_targetFilePath, Struct::AssetBrowserEditorWindowRenameState& a_renameState) const
{

}