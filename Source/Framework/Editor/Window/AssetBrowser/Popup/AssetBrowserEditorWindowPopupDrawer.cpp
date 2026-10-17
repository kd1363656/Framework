#include "AssetBrowserEditorWindowPopupDrawer.h"
#include "../../../../../Application/Application.h"

void FWK::Editor::AssetBrowserEditorWindowPopupDrawer::BeginPopup(const std::string_view& a_openPopupLabel) const
{
    if (a_openPopupLabel.empty()) { return; }

    ImGui::OpenPopup(a_openPopupLabel.data());
}

void FWK::Editor::AssetBrowserEditorWindowPopupDrawer::Draw(const std::vector<std::filesystem::path>& a_selectedFilePathList, 
                                                            const std::filesystem::path&              a_targetFilePath,
                                                            const std::string_view&                   a_openPopupLabel,
                                                            const Enum::AssetBrowserPopupContextType  a_contextType,
                                                                  AssetBrowserEditorWindow&           a_editorWindow)
{
    if (!ImGui::BeginPopup(a_openPopupLabel.data())) { return; }

    // 上下キーとマウスで項目を選べるようにする
    m_popupNavigator.Begin(Enum::EditorPopupNavigationAxis::Vertical);

    const auto& l_constClipboard        = a_editorWindow.GetREFClipboard                  ();
    const auto& l_assetCreator          = a_editorWindow.GetREFAssetCreator               ();
          auto& l_clipboard             = a_editorWindow.GetMutableREFClipboard           ();
          auto& l_application           = Application::GetInstance                        ();
          auto& l_assetFilePathRegistry = l_application.GetMutableREFAssetFilePathRegistry();
    const auto& l_fileOperation         = a_editorWindow.GetREFFileOperation              ();
          auto& l_renameState           = a_editorWindow.GetMutableREFRenameState         ();

    // AssetPaneの空白右クリックかどうか
    const bool l_isAssetPaneEmpty = a_contextType == Enum::AssetBrowserPopupContextType::AssetPane_OnEmpty;

    // 複数選択中かどうか
    // 複数選択中は名前変更・新規フォルダ作成を無効にする
    const bool l_isMultiSelection = a_selectedFilePathList.size() > Constant::k_editorSelectedFolderSingleSize;

    // 対象フォルダがルートフォルダかどうか
    const bool l_isRootFolder = a_targetFilePath == Constant::k_assetRootFolderPath;

    // 選択中リストにルートフォルダが含まれているかどうか
    const bool l_containsRoot = std::find(a_selectedFilePathList.begin(), a_selectedFilePathList.end(), Constant::k_assetRootFolderPath) != a_selectedFilePathList.end();

    // 新規フォルダ作成可能かどうか
    const bool l_canCreateFolder = !l_isMultiSelection                                                       &&
                                   (a_contextType == Enum::AssetBrowserPopupContextType::FolderPane_OnFolder ||
                                    a_contextType == Enum::AssetBrowserPopupContextType::AssetPane_OnFolder  ||
                                    a_contextType == Enum::AssetBrowserPopupContextType::AssetPane_OnEmpty);

    // 名前変更可能か
    // OnFolder(両Pane)・OnFile(AssetPane)で可能
    // OnEmptyでは対象がないため不可
    // 複数選択中は無効
    const bool l_canRename = !l_isMultiSelection                                                       &&
                             !l_isRootFolder                                                           &&
                             (a_contextType == Enum::AssetBrowserPopupContextType::FolderPane_OnFolder ||
                              a_contextType == Enum::AssetBrowserPopupContextType::AssetPane_OnFolder  ||
                              a_contextType == Enum::AssetBrowserPopupContextType::AssetPane_OnFile);

    // 選択中のファイルがあるか(コピー / 切り取り / 複製 / 削除の判定に使用)
    const bool l_hasSelection = !a_selectedFilePathList.empty();

    // クリップボードが空でないか(貼り付けの判定に使用)
    // Clipboard::IsEmpty()はconst参照で調べる
    const bool l_canPaste = !l_constClipboard.IsEmpty();

    // 新規フォルダ
    DrawCreateFolderMenu(a_targetFilePath,
                         a_contextType,
                         l_canCreateFolder,
                         a_editorWindow);

    // 新規プレハブ(AssetPane_OnEmptyのみ表示)
    if (l_isAssetPaneEmpty &&
        !l_isMultiSelection)
    {
        DrawCreatePrefabMenu(a_targetFilePath,
                             l_assetCreator,
                             true,
                             l_assetFilePathRegistry,
                             l_renameState);
    }

    // 新規シーン(AssetPane_OnEmptyのみ表示)
    if (l_isAssetPaneEmpty &&
        !l_isMultiSelection)
    {
        DrawCreateSceneMenu(a_targetFilePath,
                            l_assetCreator,
                            true,
                            l_assetFilePathRegistry,
                            l_renameState);
    }

    // 操作項目
    if (!l_isAssetPaneEmpty)
    {
        // ImGui::Separatorで作成項目と走査項目の間に区切り線を引く
        ImGui::Separator();

        DrawRenameMenu(a_targetFilePath, l_canRename, l_renameState);

        DrawCopyMenu(a_selectedFilePathList, 
                     l_fileOperation,
                     l_hasSelection &&
                     !l_containsRoot,
                     l_clipboard);

        DrawCutMenu(a_selectedFilePathList,
                    l_fileOperation,
                    l_hasSelection &&
                    !l_containsRoot,
                    l_clipboard);

        DrawPasteMenu(a_selectedFilePathList, 
                      l_fileOperation,
                      l_canPaste,
                      l_clipboard);

        DrawDuplicateMenu(a_selectedFilePathList,
                          l_fileOperation,
                          l_hasSelection &&
                          !l_containsRoot);

        DrawDeleteMenu(a_selectedFilePathList,
                       l_hasSelection &&
                       !l_containsRoot,
                       a_editorWindow);
    }
    else
    {
        // AssetPane_OnEmpty(空白右クリック)の場合
        // 作成系メニュー(新規フォルダ/プレハブ/シーン)の後に区切り線を引き
        // 貼り付けのみ表示する
        ImGui::Separator();

        DrawPasteMenu(a_selectedFilePathList,
                      l_fileOperation,
                      l_canPaste,
                      l_clipboard);
    }

    m_popupNavigator.End();

    ImGui::EndPopup();
}

void FWK::Editor::AssetBrowserEditorWindowPopupDrawer::DrawCreateFolderMenu(const std::filesystem::path&             a_targetFolderPath, 
                                                                            const Enum::AssetBrowserPopupContextType a_contextPopup,
                                                                            const bool                               a_canCreate, 
                                                                                  AssetBrowserEditorWindow&          a_assetBrowserEditorWindow)
{
    // アイコン + ラベル文字列を構築
          auto& l_folderPane   = a_assetBrowserEditorWindow.GetMutableREFFolderPane ();
    const auto& l_assetCreator = a_assetBrowserEditorWindow.GetREFAssetCreator      ();
          auto& l_renameState  = a_assetBrowserEditorWindow.GetMutableREFRenameState();

    // m_popupNavigator.MenuItem(ラベル、
    //                 ショートカット文字列、
    //                 選択状態、
    //                 有効/無効(falseでグレーアウト))
    // MenuItem(label, 
    //          shortcut,
    //          selected,
    //          enabled)
    // enabled = falseの場合、項目はグレーアウトされクリックしても反応しない
    if (const auto& l_label = std::string{ k_imguiFontAwesomeFolderPlusIcon } + " " + std::string{ k_createNewFolderLabel };
        m_popupNavigator.MenuItem(l_label.c_str(), k_createNewFolderShortcutLabel.data(), a_canCreate))
    {
        if (const auto& l_result = l_assetCreator.CreateFolder(a_targetFolderPath);
            l_result.m_isSuccess)
        {
            // 作成したフォルダがツリーに見えるように
            // 親フォルダを開状態にする
            // これを行わないと親ノードが閉じたままで
            // 新規フォルダ(とリネーム用InputText)が表示されない
            // AssetPaneからの作成の場合もFolderPaneツリーの親を開いておくことで
            // FolderPaneに切り替えた時に展開された状態で表示される
            l_folderPane.ApplyFolderOpenState(a_targetFolderPath, true);

            // 作成したフォルダを現在選択中のファイルパスにする
            // 選択状態になることでハイライト表示され
            // 次の操作(コピー/切り取り/複製等)の対象になる
            auto& l_selectionState = l_folderPane.GetMutableREFSelectionState();
            
            // コンテキストに応じて現在参照中フォルダパス(m_currentSelectFolderPath)を制御
            // AssetPane_OnEmpty   : 変更しない(現在のフォルダに留まり新規フォルダをカード表示)
            // AssetPane_OnFolder  : 作成先(選択フォルダ)へ移動し新規フォルダをカード表示
            //                       AssetPaneのDrawCardRenameでリネームする
            // FolderPane_OnFolder : 新規フォルダへ移動しFolderPaneツリーでリネームする
            if (a_contextPopup == Enum::AssetBrowserPopupContextType::AssetPane_OnFolder)
            {
                // 作成先(選択フォルダ)を現在参照中フォルダにする
                // 新規フォルダがそのフォルダ内にカードとして表示され
                // AssetPaneのDrawCardRenameでリネーム可能になる
                a_assetBrowserEditorWindow.SetCurrentSelectFolderPath(a_targetFolderPath);

                // 新規フォルダを選択(現在参照中フォルダは上で設定済みなので更新しない)
                l_selectionState.SelectSingleFolder(l_result.m_createdFilePath, a_assetBrowserEditorWindow, false);
            }
            else
            {
                // AssetPaneOn_Empty    : 現在参照中フォルダを変更しない            (false)
                // FolderPane_OnFolder : 新規フォルダを現在参照中のファイルダにする(true)
                const bool l_updateCurrentFolderPath = a_contextPopup != Enum::AssetBrowserPopupContextType::AssetPane_OnEmpty;

                l_selectionState.SelectSingleFolder(l_result.m_createdFilePath, a_assetBrowserEditorWindow, l_updateCurrentFolderPath);
            }

            // 作成性孤児、名前変更モードへ移行
            // ユーザーがすぐにフォルダ名を編集できるようにする
            StartRename(l_result.m_createdFilePath, l_renameState);
        }
    }  
}
void FWK::Editor::AssetBrowserEditorWindowPopupDrawer::DrawCreatePrefabMenu(const std::filesystem::path&                       a_targetFolderPath,
                                                                            const AssetBrowserEditorWindowAssetCreator&        a_assetCreator, 
                                                                            const bool                                         a_canCreate,
                                                                                  AssetFilePathRegistry&                       a_assetFilePathRegistry, 
                                                                                  Struct::AssetBrowserEditorWindowRenameState& a_renameState)
{
    if (const auto& l_label = std::string{ Constant::k_imguiFontAwesomePrefabIcon } + " " + std::string{ k_createNewPrefabLabel };
        m_popupNavigator.MenuItem(l_label.c_str(), nullptr, a_canCreate))
    {
        // AssetCreator::CreatePrefabでプレハブファイルを作成
        // Prefab作成にはAssetFilePathRegistryが必要(UUID登録のため)
        if (const auto& l_result = a_assetCreator.CreatePrefab(a_targetFolderPath, a_assetFilePathRegistry);
            l_result.m_isSuccess)
        {
            StartRename(l_result.m_createdFilePath, a_renameState);
        }
    }
}
void FWK::Editor::AssetBrowserEditorWindowPopupDrawer::DrawCreateSceneMenu(const std::filesystem::path&                       a_targetFolderPath, 
                                                                           const AssetBrowserEditorWindowAssetCreator&        a_assetCreator, 
                                                                           const bool                                         a_canCreate,
                                                                                 AssetFilePathRegistry&                       a_assetFilePathRegistry,
                                                                                 Struct::AssetBrowserEditorWindowRenameState& a_renameState)
{
    if (const auto& l_label = std::string{ Constant::k_imguiFontAwesomeSceneIcon } + " " + std::string{ k_createNewSceneLabel };
        m_popupNavigator.MenuItem(l_label.c_str(), nullptr, a_canCreate))
    {
        // AssetCreator::CreateSceneでシーンファイルを作成
        // Scene作成にもAssetFilePathRegistryが必要(UUID登録のため)
        if (const auto& l_result = a_assetCreator.CreateScene(a_targetFolderPath, a_assetFilePathRegistry);
            l_result.m_isSuccess)
        {
            StartRename(l_result.m_createdFilePath, a_renameState);
        }
    }
}

void FWK::Editor::AssetBrowserEditorWindowPopupDrawer::DrawRenameMenu(const std::filesystem::path& a_targetFilePath, const bool a_canRename, Struct::AssetBrowserEditorWindowRenameState& a_renameState)
{
    // ショートカット : F2
    // a_canRename = falseの場合、グレーアウトされクリックしても反応しない
    if (const auto& l_label = std::string{ k_imguiFontAwesomeEditIcon } + " " + std::string{ k_renameLabel };
        m_popupNavigator.MenuItem(l_label.c_str(), k_renameShortcutLabel.data(), a_canRename))
    {
        // 対象ファイルの名前変更モードを開始
        StartRename(a_targetFilePath, a_renameState);
    }
}
void FWK::Editor::AssetBrowserEditorWindowPopupDrawer::DrawCopyMenu(const std::vector<std::filesystem::path>&    a_selectedFilePathList, 
                                                                    const AssetBrowserEditorWindowFileOperation& a_fileOperation,
                                                                    const bool                                   a_hasSelection, 
                                                                          AssetBrowserEditorWindowClipboard&     a_clipboard)
{
    // ショートカット : Ctrl + C
    // 選択中のファイルがない場合はグレーアウト
    if (const auto& l_label = std::string{ k_imguiFontAwesomeCopyIcon } + " " + std::string{ k_copyLabel };
        m_popupNavigator.MenuItem(l_label.c_str(), k_copyShortcutLabel.data(), a_hasSelection))
    {
        // FileOperation::Copyで選択中のファイルをクリックボードへコピー
        // コピー元ファイルは削除されない
        a_fileOperation.Copy(a_selectedFilePathList, a_clipboard);
    }

}
void FWK::Editor::AssetBrowserEditorWindowPopupDrawer::DrawCutMenu(const std::vector<std::filesystem::path>&    a_selectedFilePathList, 
                                                                   const AssetBrowserEditorWindowFileOperation& a_fileOperation, 
                                                                   const bool                                   a_hasSelection,
                                                                         AssetBrowserEditorWindowClipboard&     a_clipboard)
{
    // ショートカット : Ctrl + X
    if (const auto& l_label = std::string{ k_imguiFontAwesomeCutIcon } + " " + std::string{ k_cutLabel };
        m_popupNavigator.MenuItem(l_label.c_str(), k_cutShortcutLabel.data(), a_hasSelection))
    {
        // FileOperation::Cutで選択中のファイルをクリックボードへ切り取り
        // 貼り付け時に元ファイルが削除される
        a_fileOperation.Cut(a_selectedFilePathList, a_clipboard);
    }
}
void FWK::Editor::AssetBrowserEditorWindowPopupDrawer::DrawPasteMenu(const std::vector<std::filesystem::path>&    a_selectedFilePathList,
                                                                     const AssetBrowserEditorWindowFileOperation& a_fileOperation, 
                                                                     const bool                                   a_canPaste, 
                                                                           AssetBrowserEditorWindowClipboard&     a_clipboard)
{
    // ショートカット : Ctrl + V
    // クリップボードが空の場合はグレーアウト
    if (const auto& l_label = std::string{ k_imguiFontAwesomePasteIcon } + " " + std::string{ k_pasteLabel };
        m_popupNavigator.MenuItem(l_label.c_str(), k_pasteShortcutLabel.data(), a_canPaste))
    {
        // FileOperation::Pasteでクリップボードのファイルを現在フォルダへ貼り付け
        // Cutの場合は元ファイルを削除、Copyの場合は複製
        a_fileOperation.Paste(a_selectedFilePathList, a_clipboard);
    }
}
void FWK::Editor::AssetBrowserEditorWindowPopupDrawer::DrawDuplicateMenu(const std::vector<std::filesystem::path>& a_selectedFilePathList, const AssetBrowserEditorWindowFileOperation& a_fileOperation, const bool a_hasSelection)
{
    // ショートカット : Ctrl + D
    // 選択中のファイルがない場合はグレーアウト
    if (const auto& l_label = std::string{ k_imguiFontAwesomeCloneIcon } + " " + std::string{ k_duplicateLabel };
        m_popupNavigator.MenuItem(l_label.c_str(), k_duplicateShortcutLabel.data(), a_hasSelection))
    {
        // FileOperation::Duplicateで選択中のファイルを複製
        // 同名の場合は自動で番号付与される(Player -> Player1)
        a_fileOperation.Duplicate(a_selectedFilePathList);
    }
}
void FWK::Editor::AssetBrowserEditorWindowPopupDrawer::DrawDeleteMenu(const std::vector<std::filesystem::path>& a_selectedFilePathList, const bool a_hasSelection, AssetBrowserEditorWindow& a_editorWindow)
{
    // ショートカット : Del
    // 選択中のファイルがない場合はグレーアウト
    if (const auto& l_label = std::string{ k_imguiFontAwesomeTrashIcon } + " " + std::string{ k_deleteLabel };
        m_popupNavigator.MenuItem(l_label.c_str(), k_deleteShortcutLabel.data(), a_hasSelection))
    {
        // FileOperation::Deleteを直接呼ばず
        // 削除確認ダイアログをリクエストする
        const auto& l_deleteConfirmPopup = a_editorWindow.GetREFDeleteConfirmPopup();

        l_deleteConfirmPopup.Request(a_selectedFilePathList, a_editorWindow);
    }
}

void FWK::Editor::AssetBrowserEditorWindowPopupDrawer::StartRename(const std::filesystem::path& a_targetFilePath, Struct::AssetBrowserEditorWindowRenameState& a_renameState) const
{
    // 名前変更対象のパスを設定
    a_renameState.m_targetFilePath = a_targetFilePath;

    // InputTextの初期値としてファイル名(拡張子なし)を入れ、名前変更モードをアクティブにする
    // 例 "NewFolder"      -> "NewFolder"
    //    "NewPrefab.json" -> "NewPrefab"
    // 呼び出し側(Pane)は次Frame以降このフラグを確認し
    // 通常のファイル名描画の代わりにImGui::InputTextを描画する
    Utility::StartEditorRename(a_targetFilePath.stem().string(), a_renameState);
}