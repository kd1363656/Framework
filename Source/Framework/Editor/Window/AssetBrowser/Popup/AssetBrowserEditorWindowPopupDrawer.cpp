#include "AssetBrowserEditorWindowPopupDrawer.h"

void FWK::Editor::AssetBrowserEditorWindowPopupDrawer::BeginPopup(const std::string_view& a_openPopupLabel) const
{
    if (a_openPopupLabel.empty()) { return; }

    ImGui::OpenPopup(a_openPopupLabel.data());
}

void FWK::Editor::AssetBrowserEditorWindowPopupDrawer::Draw(const std::vector<std::filesystem::path>& a_selectedFilePathList, 
                                                            const std::filesystem::path&              a_targetFilePath,
                                                            const std::string_view&                   a_openPopupLabel,
                                                            const Enum::AssetBrowserPopupContextType  a_contextType,
                                                                  AssetBrowserEditorWindow&           a_editorWindow) const
{
    if (!ImGui::BeginPopup(a_openPopupLabel.data())) { return; }

    const auto& l_constClipboard         = a_editorWindow.GetREFClipboard                   ();
    const auto& l_assetCreator           = a_editorWindow.GetREFAssetCreator                ();
          auto& l_clipboard              = a_editorWindow.GetMutableREFClipboard            ();
          auto& l_assetFilePathRegistry  = a_editorWindow.GetMutableREFAssetFilePathRegistry();
          auto& l_fileOperation          = a_editorWindow.GetMutableREFFileOperation        ();
          auto& l_renameState            = a_editorWindow.GetMutableREFRenameState          ();

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
    const bool l_canCreateFolder = !l_isMultiSelection &&
                                   (a_contextType == Enum::AssetBrowserPopupContextType::FolderPane_OnFolder ||
                                    a_contextType == Enum::AssetBrowserPopupContextType::AssetPane_OnFolder ||
                                    a_contextType == Enum::AssetBrowserPopupContextType::AssetPane_OnEmpty);

    // 新規プレハブ・新規シーンが作成可能か
    const bool l_canCreatePrefab = l_isAssetPaneEmpty && !l_isMultiSelection;
    const bool l_canCreateScene  = l_isAssetPaneEmpty && !l_isMultiSelection;

    // 名前変更可能か
    // OnFolder(両Pane)・OnFile(AssetPane)で可能
    // OnEmptyでは対象がないため不可
    // 複数選択中は無効
    const bool l_canRename = !l_isMultiSelection &&
                             !l_isRootFolder     &&
                             (a_contextType == Enum::AssetBrowserPopupContextType::FolderPane_OnFolder ||
                              a_contextType == Enum::AssetBrowserPopupContextType::AssetPane_OnFolder  ||
                              a_contextType == Enum::AssetBrowserPopupContextType::AssetPane_OnFile);

    // 選択中のファイルがあるか(コピー / 切り取り / 複製 / 削除の判定に使用)
    const bool l_hasSelection = !a_selectedFilePathList.empty();

    // クリップボードが空でないか(貼り付けの判定に使用)
    // Clipboard::IsEmpty()はconst参照で調べる
    const bool l_canPaste = !l_constClipboard.IsEmpty();

    // 新規フォルダ
    DrawCreateFolderMenu(a_targetFilePath, l_canCreateFolder, a_editorWindow);

    // 新規プレハブ(AssetPane_OnEmptyのみ表示)
    if (l_canCreatePrefab)
    {
        DrawCreatePrefabMenu(l_assetCreator,
                             a_targetFilePath,
                             true,
                             l_assetFilePathRegistry,
                             l_renameState);
    }

    // 新規シーン(AssetPane_OnEmptyのみ表示)
    if (l_canCreateScene)
    {
        DrawCreateSceneMenu(l_assetCreator,
                            a_targetFilePath,
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
                     l_hasSelection && !l_containsRoot,
                     l_fileOperation,
                     l_clipboard);

        DrawCutMenu(a_selectedFilePathList,
                    l_hasSelection && !l_containsRoot,
                    l_fileOperation,
                    l_clipboard);

        DrawPasteMenu(a_targetFilePath, 
                      l_canPaste,
                      l_fileOperation,
                      l_clipboard);

        DrawDuplicateMenu(a_selectedFilePathList, l_hasSelection && !l_containsRoot, l_fileOperation);
        DrawDeleteMenu   (a_selectedFilePathList, l_hasSelection && !l_containsRoot, a_editorWindow);
    }

    ImGui::EndPopup();
}

void FWK::Editor::AssetBrowserEditorWindowPopupDrawer::DrawCreateFolderMenu(const std::filesystem::path& a_targetFolderPath, const bool a_canCreate, AssetBrowserEditorWindow& a_assetBrowserEditorWindow) const
{
    // アイコン + ラベル文字列を構築
    const auto& l_label        = std::string{ Constant::k_imguiFontAwesomeFolderPlusIcon } + " " + std::string{ k_createNewFolderLabel };
          auto& l_folderPane   = a_assetBrowserEditorWindow.GetMutableREFFolderPane ();
          auto& l_assetCreator = a_assetBrowserEditorWindow.GetREFAssetCreator      ();
          auto& l_renameState  = a_assetBrowserEditorWindow.GetMutableREFRenameState();

    // ImGui::MenuItem(ラベル、
    //                 ショートカット文字列、
    //                 選択状態、
    //                 有効/無効(falseでグレーアウト))
    // MenuItem(label, 
    //          shortcut,
    //          selected,
    //          enabled)
    // enabled = falseの場合、項目はグレーアウトされクリックしても反応しない
    if (ImGui::MenuItem(l_label.c_str(), 
                        k_createNewFolderShortcutLabel.data(),
                        false, 
                        a_canCreate))
    {
        const auto& l_result = l_assetCreator.CreateFolder(a_targetFolderPath);

        if (l_result.m_isSuccess)
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
            l_folderPane.SelectSingleFolder(l_result.m_createdFilePath, a_assetBrowserEditorWindow);

            // 作成性孤児、名前変更モードへ移行
            // ユーザーがすぐにフォルダ名を編集できるようにする
            StartRename(l_result.m_createdFilePath, l_renameState);
        }
    }  
}
void FWK::Editor::AssetBrowserEditorWindowPopupDrawer::DrawCreatePrefabMenu(const AssetBrowserEditorWindowAssetCreator&        a_assetCreator,
                                                                            const std::filesystem::path&                       a_targetFolderPath, 
                                                                            const bool                                         a_canCreate,
                                                                                  AssetFilePathRegistry&                       a_assetFilePathRegistry, 
                                                                                  Struct::AssetBrowserEditorWindowRenameState& a_renameState) const
{
    const auto& l_label = std::string{ Constant::k_imguiFontAwesomeCubeIcon } + " " + std::string{ k_createNewPrefabLabel };

    if (ImGui::MenuItem(l_label.c_str(), 
                        nullptr,
                        false,
                        a_canCreate))
    {
        // AssetCreator::CreatePrefabでプレハブファイルを作成
        // Prefab作成にはAssetFilePathRegistryが必要(UUID登録のため)
        const auto l_result = a_assetCreator.CreatePrefab(a_targetFolderPath, a_assetFilePathRegistry);

        if (l_result.m_isSuccess)
        {
            StartRename(l_result.m_createdFilePath, a_renameState);
        }
    }
}
void FWK::Editor::AssetBrowserEditorWindowPopupDrawer::DrawCreateSceneMenu(const AssetBrowserEditorWindowAssetCreator&        a_assetCreator, 
                                                                           const std::filesystem::path&                       a_targetFolderPath, 
                                                                           const bool                                         a_canCreate,
                                                                                 AssetFilePathRegistry&                       a_assetFilePathRegistry,
                                                                                 Struct::AssetBrowserEditorWindowRenameState& a_renameState) const
{
    const auto& l_label = std::string{ Constant::k_imguiFontAwesomeFileIcon } + " " + std::string{ k_createNewSceneLabel };

    if (ImGui::MenuItem(l_label.c_str(), 
                        nullptr,
                        false,
                        a_canCreate))
    {
        // AssetCreator::CreateSceneでシーンファイルを作成
        // Scene作成にもAssetFilePathRegistryが必要(UUID登録のため)
        const auto& l_result = a_assetCreator.CreateScene(a_targetFolderPath, a_assetFilePathRegistry);

        if (l_result.m_isSuccess)
        {
            StartRename(l_result.m_createdFilePath, a_renameState);
        }
    }
}

void FWK::Editor::AssetBrowserEditorWindowPopupDrawer::DrawRenameMenu(const std::filesystem::path& a_targetFilePath, const bool a_canRename, Struct::AssetBrowserEditorWindowRenameState& a_renameState) const
{
    // ショートカット : F2
    // a_canRename = falseの場合、グレーアウトされクリックしても反応しない
    const auto& l_label = std::string{ Constant::k_imguiFontAwesomeEditIcon } + " " + std::string{ k_renameLabel };
    
    if (ImGui::MenuItem(l_label.c_str(),
                        k_renameShortcutLabel.data(),
                        false,
                        a_canRename))
    {
        // 対象ファイルの名前変更モードを開始
        StartRename(a_targetFilePath, a_renameState);
    }
}
void FWK::Editor::AssetBrowserEditorWindowPopupDrawer::DrawCopyMenu(const std::vector<std::filesystem::path>&    a_selectedFilePathList, 
                                                                    const bool                                   a_hasSelection, 
                                                                          AssetBrowserEditorWindowFileOperation& a_fileOperation,
                                                                          AssetBrowserEditorWindowClipboard&     a_clipboard) const
{
    // ショートカット : Ctrl + C
    // 選択中のファイルがない場合はグレーアウト
    const auto& l_label = std::string{ Constant::k_imguiFontAwesomeCopyIcon } + " " + std::string{ k_copyLabel };
    
    if (ImGui::MenuItem(l_label.c_str(), 
                        k_copyShortcutLabel.data(),
                        false,
                        a_hasSelection))
    {
        // FileOperation::Copyで選択中のファイルをクリックボードへコピー
        // コピー元ファイルは削除されない
        a_fileOperation.Copy(a_selectedFilePathList, a_clipboard);
    }

}
void FWK::Editor::AssetBrowserEditorWindowPopupDrawer::DrawCutMenu(const std::vector<std::filesystem::path>&    a_selectedFilePathList, 
                                                                   const bool                                   a_hasSelection,
                                                                         AssetBrowserEditorWindowFileOperation& a_fileOperation, 
                                                                         AssetBrowserEditorWindowClipboard&     a_clipboard) const
{
    // ショートカット : Ctrl + X
    const auto& l_label = std::string{ Constant::k_imguiFontAwesomeCutIcon } + " " + std::string{ k_cutLabel };
    
    if (ImGui::MenuItem(l_label.c_str(), 
                        k_cutShortcutLabel.data(),
                        false,
                        a_hasSelection))
    {
        // FileOperation::Cutで選択中のファイルをクリックボードへ切り取り
        // 貼り付け時に元ファイルが削除される
        a_fileOperation.Cut(a_selectedFilePathList, a_clipboard);
    }
}
void FWK::Editor::AssetBrowserEditorWindowPopupDrawer::DrawPasteMenu(const std::filesystem::path&                 a_targetFolderPath, 
                                                                     const bool                                   a_canPaste, 
                                                                           AssetBrowserEditorWindowFileOperation& a_fileOperation, 
                                                                           AssetBrowserEditorWindowClipboard&     a_clipboard) const
{
    // ショートカット : Ctrl + V
    // クリップボードが空の場合はグレーアウト
    const auto& l_label = std::string{ Constant::k_imguiFontAwesomePasteIcon } + " " + std::string{ k_pasteLabel };
    
    if (ImGui::MenuItem(l_label.c_str(), 
                        k_pasteShortcutLabel.data(),
                        false,
                        a_canPaste))
    {
        // FileOperation::Pasteでクリップボードのファイルを現在フォルダへ貼り付け
        // Cutの場合は元ファイルを削除、Copyの場合は複製
        a_fileOperation.Paste(a_targetFolderPath, a_clipboard);
    }
}
void FWK::Editor::AssetBrowserEditorWindowPopupDrawer::DrawDuplicateMenu(const std::vector<std::filesystem::path>& a_selectedFilePathList, const bool a_hasSelection, AssetBrowserEditorWindowFileOperation& a_fileOperation) const
{
    // ショートカット : Ctrl + D
    // 選択中のファイルがない場合はグレーアウト
    const auto& l_label = std::string{ Constant::k_imguiFontAwesomeCloneIcon } + " " + std::string{ k_duplicateLabel };
    
    if (ImGui::MenuItem(l_label.c_str(), 
                        k_duplicateShortcutLabel.data(),
                        false,
                        a_hasSelection))
    {
        // FileOperation::Duplicateで選択中のファイルを複製
        // 同名の場合は自動で番号付与される(Player -> Player1)
        a_fileOperation.Duplicate(a_selectedFilePathList);
    }
}
void FWK::Editor::AssetBrowserEditorWindowPopupDrawer::DrawDeleteMenu(const std::vector<std::filesystem::path>& a_selectedFilePathList, const bool a_hasSelection, AssetBrowserEditorWindow& a_editorWindow) const
{
    // ショートカット : Del
    // 選択中のファイルがない場合はグレーアウト
    const auto& l_label = std::string{ Constant::k_imguiFontAwesomeTrashIcon } + " " + std::string{ k_deleteLabel };
    
    if (ImGui::MenuItem(l_label.c_str(), 
                        k_deleteShortcutLabel.data(),
                        false,
                        a_hasSelection))
    {
        // FileOperation::Deleteを直接呼ばず
        // 削除確認ダイアログをリクエストする
        auto& l_deleteConfirmPopup = a_editorWindow.GetREFDeleteConfirmPopup();

        l_deleteConfirmPopup.Request(a_selectedFilePathList, a_editorWindow);
    }
}

void FWK::Editor::AssetBrowserEditorWindowPopupDrawer::StartRename(const std::filesystem::path& a_targetFilePath, Struct::AssetBrowserEditorWindowRenameState& a_renameState) const
{
    // 名前変更対象のパスを設定
    a_renameState.m_targetFilePath = a_targetFilePath;

    // 名前変更モードをアクティブにする
    // 呼び出し側(Pane)は次Frame以降このフラグを確認し
    // 通常のファイル名描画の代わりにImGui::InputTextを描画する
    a_renameState.m_isActive = true;

    // InputTextの初期値としてファイル名を設定
    // 例 "NewFolder"      -> "NewFolder"
    //    "NewPrefab.json" -> "NewPrefab"
    const auto& l_stem = a_targetFilePath.stem().string();

    // バッファをゼロクリア
    // std::array::fillで全要求を'\0'にする
    a_renameState.m_inputBuffer.fill(Constant::k_nullCharacter);

    // ファイル名をバッファへコピー
    // バッファサイズを超えないようにminで制限
    // -1は終端null用
    const auto l_copySize = std::min(l_stem.size(), a_renameState.m_inputBuffer.size() - Constant::k_inputBufferLastSizeOffsetForCopy);

    // std::copy_nで先頭からl_copySize分をコピー
    std::copy_n(l_stem.begin(), l_copySize, a_renameState.m_inputBuffer.begin());

    // 初回フォーカス制御フラグをリセット
    // m_isActiveがtrueになった直後はフォーカスされていない状態
    a_renameState.m_isFocused = false;
}