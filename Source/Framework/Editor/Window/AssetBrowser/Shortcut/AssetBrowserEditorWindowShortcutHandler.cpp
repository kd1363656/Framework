#include "AssetBrowserEditorWindowShortcutHandler.h"

void FWK::Editor::AssetBrowserEditorWindowShortcutHandler::Handle(const std::vector<std::filesystem::path>& a_selectedFilePathList,
                                                                  const std::filesystem::path&              a_selectedFilePath,
                                                                  const std::filesystem::path&              a_targetFilePath,
                                                                        AssetBrowserEditorWindow&           a_editorWindow) const
{
    // アクティブPane無効の場合は何もしない
    if (const auto l_activePane = a_editorWindow.GetVALActivePane();
        l_activePane == Enum::AssetBrowserActivePaneType::Invalid)
    {
        return;
    }

    // このフレームで押されたショートカットを取得する
    // キーの割り当てはWorldOutlinerなど他のEditorWindowと共通(EditorWindowShortcutUtilityConstant.h)
    const auto l_shortcutType = Utility::FetchVALPressedEditorWindowShortcut();

    if (l_shortcutType == Enum::EditorWindowShortcutType::None) { return; }

    // 選択中のファイルがあるか
    const bool l_hasSelection = !a_selectedFilePathList.empty();

    // 単一選択かどうか(名前変更は単一選択時のみ有効)
    const bool l_isSingleSelection = (a_selectedFilePathList.size() == Constant::k_editorSelectedFolderSingleSize);

    // 複数選択中かどうか
    // 複数選択中は名前変更・新規フォルダ作成を無効にする
    const bool l_isMultiSelection = a_selectedFilePathList.size() > Constant::k_editorSelectedFolderSingleSize;

    // クリップボードが空でないか(貼り付けの判定に使用)
          auto& l_clipboard     = a_editorWindow.GetMutableREFClipboard  ();
          auto& l_renameState   = a_editorWindow.GetMutableREFRenameState();
    const auto& l_fileOperation = a_editorWindow.GetREFFileOperation     ();
    const bool  l_canPaste      = !l_clipboard.IsEmpty                   ();

    // リネーム対策がルートフォルダかどうか
    // PopupDrawerのl_isRootFolderと同じ意味にするため
    // リネーム対象であるa_selectedFilePathで判定する
    // (FolderPaneではa_selectedFilePath == a_targetFilePathのため従来通り)
    // (AssetPaneでは選択ファイルがルート化で判定する、ルートはカード表示されないため通常false)
    const bool l_isRootFolder = a_targetFilePath == Constant::k_assetRootFolderPath;

    // 選択中リストにルートフォルダが含まれているかどうか
    // ルートフォルダはコピー、切り取り、複製、削除できない
    const bool l_containsRoot = std::find(a_selectedFilePathList.begin(), a_selectedFilePathList.end(), Constant::k_assetRootFolderPath) != a_selectedFilePathList.end();

    // 選択中のファイルに対して操作できるか(コピー、切り取り、複製、削除で共通の条件)
    const bool l_canOperateSelection = l_hasSelection &&
                                       !l_containsRoot;

    switch (l_shortcutType)
    {
    case Enum::EditorWindowShortcutType::CreateFolder:
    {
        // Ctrl + Shift + N : 新規フォルダ作成
        // アクティブPaneと選択状態からコンテキストを判定し
        // PopupDrawerの右クリックメニューと同じ挙動にする
        // FolderPane : FolderPane_OnFolder
        // AssetPane + 未選択       : AssetPane_OnEmpty
        // AssetPane + フォルダ選択 : AssetPane_OnFolder
        // AssetPane + ファイル選択 : AssetPane_OnfFile(作成不可)
        Enum::AssetBrowserPopupContextType l_createFolderContext = Enum::AssetBrowserPopupContextType::FolderPane_OnFolder;

        if (a_editorWindow.GetVALActivePane() == Enum::AssetBrowserActivePaneType::AssetPane)
        {
            if (a_selectedFilePathList.empty())
            {
                l_createFolderContext = Enum::AssetBrowserPopupContextType::AssetPane_OnEmpty;
            }
            else
            {
                std::error_code l_errorCode = {};

                l_createFolderContext = std::filesystem::is_directory(a_selectedFilePath, l_errorCode) ? Enum::AssetBrowserPopupContextType::AssetPane_OnFolder : Enum::AssetBrowserPopupContextType::AssetPane_OnFile;
            }
        }

        // 作成可能か判定
        // AssetPane_OnFile(ファイル選択中)は作成不可
        // PopupDrawerのl_canCreateFolderと同じ条件
        if (!l_isMultiSelection                                                              &&
           (l_createFolderContext == Enum::AssetBrowserPopupContextType::FolderPane_OnFolder ||
            l_createFolderContext == Enum::AssetBrowserPopupContextType::AssetPane_OnFolder  ||
            l_createFolderContext == Enum::AssetBrowserPopupContextType::AssetPane_OnEmpty))
        {
            HandleCreateFolder(a_targetFilePath, l_createFolderContext, a_editorWindow);
        }

        break;
    }
    case Enum::EditorWindowShortcutType::Rename:
    {
        // F2 : 名前変更(単一選択時のみ有効)
        if (l_isSingleSelection &&
            !l_isRootFolder)
        {
            HandleRename(a_selectedFilePath, l_renameState);
        }

        break;
    }
    case Enum::EditorWindowShortcutType::Copy:
    {
        // Ctrl + C : コピー
        if (l_canOperateSelection) { HandleCopy(a_selectedFilePathList, l_fileOperation, l_clipboard); }

        break;
    }
    case Enum::EditorWindowShortcutType::Cut:
    {
        // Ctrl + X : 切り取り
        if (l_canOperateSelection) { HandleCut(a_selectedFilePathList, l_fileOperation, l_clipboard); }

        break;
    }
    case Enum::EditorWindowShortcutType::Paste:
    {
        // Ctrl + V : 貼り付け(クリップボードが空でない場合のみ)
        // Windows Explorerと同じく現在開いているフォルダへ張り付ける
        if (l_canPaste) { HandlePaste(a_selectedFilePathList, l_fileOperation, l_clipboard); }

        break;
    }
    case Enum::EditorWindowShortcutType::Duplicate:
    {
        // Ctrl + D : 複製
        if (l_canOperateSelection) { HandleDuplicate(a_selectedFilePathList, l_fileOperation); }

        break;
    }
    case Enum::EditorWindowShortcutType::Delete:
    {
        // Delete : 削除(確認ダイアログを表示する)
        if (l_canOperateSelection) { HandleDelete(a_selectedFilePathList, a_editorWindow); }

        break;
    }
    default:
    {
        // SelectAllはPaneごとに対象が違うため、HandleFolderPane / HandleAssetPaneで処理する
        break;
    }
    }
}
void FWK::Editor::AssetBrowserEditorWindowShortcutHandler::HandleFolderPane(AssetBrowserEditorWindow& a_editorWindow)
{
    auto& l_folderPane = a_editorWindow.GetMutableREFFolderPane();

    // Ctrl + A : 現在フォルダツリーで表示中のフォルダをすべて選択する
    // FetchVALDisplayedFolderList()で表示中フォルダリストを取得し
    // SelectionState::SelectAllで全選択する
    // ルートフォルダ(Asset)も選択対象に含まれるが
    // 既存のHandle側でルートフォルダ含む場合は操作無効化されるため問題ない
    if (Utility::FetchVALPressedEditorWindowShortcut() == Enum::EditorWindowShortcutType::SelectAll)
    {
        // 表示中フォルダリストを取得
        // Assetルーっとから再帰的においているフォルダの子を収集し
        // 表示順(上から下に)並べたリスト
        const auto& l_displayedFolderList = l_folderPane.FetchVALDisplayedFolderList(a_editorWindow);

        // SelectionStateを取得して全選択を実行
        // SelectAll内部で選択リストをクリアしてかrあ
        // 表示リスト全要素を追加する
        auto& l_selectionState = l_folderPane.GetMutableREFSelectionState();

        l_selectionState.SelectAll(l_displayedFolderList);
    }

    // 上下キーによる操作の反映
    // Shift又はCtrlを押していれば範囲選択(WorldOutlinerと共通)
    const bool l_isRangeSelection = Utility::IsEditorWindowRangeSelectionKeyDown();

    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
    {
        l_folderPane.MoveSelectionUp(a_editorWindow, l_isRangeSelection);
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
    {
        l_folderPane.MoveSelectionDown(a_editorWindow, l_isRangeSelection);
    }

    // 右キー左キーによる操作の反映
    if (ImGui::IsKeyPressed(ImGuiKey_RightArrow, false))
    {
        l_folderPane.ForciblyFolderOpen(a_editorWindow);
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow, false))
    {
        l_folderPane.ForciblyFolderClose(a_editorWindow);
    }

    // エンターキーで開閉の切り替え
    if (ImGui::IsKeyPressed(ImGuiKey_Enter, false))
    {
        l_folderPane.ToggleCurrentFolderOpen(a_editorWindow);
    }
}

void FWK::Editor::AssetBrowserEditorWindowShortcutHandler::HandleAssetPane(AssetBrowserEditorWindow& a_editorWindow)
{
    auto& l_assetPane = a_editorWindow.GetMutableREFAssetPane();

    // Ctrl + A : 表示中のファイルをすべて選択
    // FetchVALDisplayedFilePathListで表示中リストを取得し
    // SelectionState::SelectAllで全選択する
    if (Utility::FetchVALPressedEditorWindowShortcut() == Enum::EditorWindowShortcutType::SelectAll)
    {
        const auto& l_displayedFilePathList = l_assetPane.FetchVALDisplayedFilePathList(a_editorWindow);

        auto& l_selectionState = l_assetPane.GetMutableREFSelectionState();

        l_selectionState.SelectAll(l_displayedFilePathList);
    }

    // 矢印キーによる操作
    // グリッドレイアウト対応
    // ↑↓ : 1行分(CardsPerRow個)移動
    // ←→ : 1カード移動
    // Shift又はCtrlを押していれば範囲選択(WorldOutlinerと共通)
    const bool l_isRangeSelection = Utility::IsEditorWindowRangeSelectionKeyDown();

    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
    {
        l_assetPane.MoveSelectionUp(a_editorWindow, l_isRangeSelection);
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
    {
        l_assetPane.MoveSelectionDown(a_editorWindow, l_isRangeSelection);
    }

    if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
    {
        l_assetPane.MoveSelectionLeft(a_editorWindow, l_isRangeSelection);
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_RightArrow))
    {
        l_assetPane.MoveSelectionRight(a_editorWindow, l_isRangeSelection);
    }

    // Enterキーでフォルダナビゲート
    // カーソル位置がフォルダの場合、そのフォルダへ移動
    if (ImGui::IsKeyPressed(ImGuiKey_Enter, false))
    {
        l_assetPane.NavigateToCurrentCursor(a_editorWindow);
    }

    // BackSpaceキーで一つ上の階層へ戻る
    if (ImGui::IsKeyPressed(ImGuiKey_Backspace, false))
    {
        l_assetPane.NavigateToFolderUp(a_editorWindow);
    }
}

void FWK::Editor::AssetBrowserEditorWindowShortcutHandler::HandleCreateFolder(const std::filesystem::path& a_parentFolderPath, const Enum::AssetBrowserPopupContextType a_contextType, AssetBrowserEditorWindow& a_editorWindow) const
{
    const auto& l_assetCreator = a_editorWindow.GetREFAssetCreator      ();
          auto& l_folderPane   = a_editorWindow.GetMutableREFFolderPane ();
          auto& l_renameState  = a_editorWindow.GetMutableREFRenameState();

    // AssetCreator::CreateFolderでフォルダを作成
    // 戻り値がCreationResultに作成パスと成否が入っている
    const auto& l_result = l_assetCreator.CreateFolder(a_parentFolderPath);

    if (!l_result.m_isSuccess) { return; }

    // 作成したフォルダがツリーに見えるように
    // 親フォルダを開状態にする
    // これを行わないと親ノードが閉じたままで
    //新規フォルダが表示されない
    l_folderPane.ApplyFolderOpenState(a_parentFolderPath, true);

    // 作成したフォルダを現在選択中のファイルパスにする
    // 選択状態になるとハイライト表示され
    // 次の走査(コピー/切り取り/複製)の対象になる
    auto& l_selectionState = l_folderPane.GetMutableREFSelectionState();

    // コンテキストに応じて現在参照中のフォルダパスを制御
    // PopupDrawer::DrawCreateFolderMenuと同じ挙動にする
    // AssetPane_OnEmpty   : 変更しない(現在のフォルダにとどまり新規フォルダをカード表示)
    // AssetPane_OnFolder  : 作成先(選択フォルダ)へ移動し新規フォルダをカード表示
    //                       AssetPaneのDrawCardRenameでリネームする
    // FolderPane_OnFolder : 新規フォルダへ移動しFolderPaneツリーでリネームする
    if (a_contextType == Enum::AssetBrowserPopupContextType::AssetPane_OnFolder)
    {
        // 作成先(選択フォルダ)を現在参照中フォルダにする
        // 新規フォルダがそのフォルダ内にカードとして表示され
        // AssetPaneのDrawCardRenameでリネーム可能になる
        a_editorWindow.SetCurrentSelectFolderPath(a_parentFolderPath);

        // 新規フォルダを選択(現在参照中フォルダは上で設定済みなので更新しない)
        l_selectionState.SelectSingleFolder(l_result.m_createdFilePath, a_editorWindow, false);
    }
    else
    {
        // AssetPane_OnEmpty   : 現在選択中フォルダを変更しない(false)
        // FolderPane_OnFolder : 新規フォルダを現在参照中フォルダにする(true)
        const bool l_updateCurrentFolderPath = a_contextType != Enum::AssetBrowserPopupContextType::AssetPane_OnEmpty;

        l_selectionState.SelectSingleFolder(l_result.m_createdFilePath, a_editorWindow, l_updateCurrentFolderPath);
    }

    // 作成成功時、名前変更モードへ移行
    // ShortcutHandlerはPopupDrawerに依存せずに独自に名前変更モードを起動する
    HandleRename(l_result.m_createdFilePath, l_renameState);
}

void FWK::Editor::AssetBrowserEditorWindowShortcutHandler::HandleRename(const std::filesystem::path& a_targetFilePath, Struct::AssetBrowserEditorWindowRenameState& a_renameState) const
{
    // 名前変更対象のパスを設定
    a_renameState.m_targetFilePath = a_targetFilePath;

    // 名前変更モードをアクティブにする
    a_renameState.m_isActive = true;

    // InputTextの初期値としてファイル名(拡張子なし)を設定
    Utility::IMGUICopyStringToInputTextBuffer(a_targetFilePath.stem().string(), a_renameState.m_inputBuffer);

    // 初回フォーカス制御用フラグをリセット
    a_renameState.m_isFocused = false;
}
void FWK::Editor::AssetBrowserEditorWindowShortcutHandler::HandleCopy(const std::vector<std::filesystem::path>& a_selectedFilePathList, const AssetBrowserEditorWindowFileOperation& a_fileOperation, TypeAlias::AssetBrowserEditorWindowClipboard& a_clipboard) const
{
    // FileOperation::Copyで選択中のファイルをクリップボードへコピー
    // コピー元ファイルは削除されない
    a_fileOperation.Copy(a_selectedFilePathList, a_clipboard);
}
void FWK::Editor::AssetBrowserEditorWindowShortcutHandler::HandleCut(const std::vector<std::filesystem::path>& a_selectedFilePathList, const AssetBrowserEditorWindowFileOperation& a_fileOperation, TypeAlias::AssetBrowserEditorWindowClipboard& a_clipboard) const
{
    // FileOperation::Cutで選択中のファイルをクリップボードへ切り取り
    // 貼り付け時に元ファイルが削除される
    a_fileOperation.Cut(a_selectedFilePathList, a_clipboard);
}
void FWK::Editor::AssetBrowserEditorWindowShortcutHandler::HandlePaste(const std::vector<std::filesystem::path>& a_selectedFilePathList, const AssetBrowserEditorWindowFileOperation& a_fileOperation, TypeAlias::AssetBrowserEditorWindowClipboard& a_clipboard) const
{
    // FileOperation::Pasteでクリップボードのファイルを現在フォルダへ張り付け
    // Cutの場合は元ファイルを削除、Copyの場合は複製
    a_fileOperation.Paste(a_selectedFilePathList, a_clipboard);
}
void FWK::Editor::AssetBrowserEditorWindowShortcutHandler::HandleDuplicate(const std::vector<std::filesystem::path>& a_selectedFilePathList, const AssetBrowserEditorWindowFileOperation& a_fileOperation) const
{
    // FileOperation::Duplicateで選択中のファイルを複製
    // 同名の場合は自動で番号付与される(Player -> Player1)
    a_fileOperation.Duplicate(a_selectedFilePathList);
}
void FWK::Editor::AssetBrowserEditorWindowShortcutHandler::HandleDelete(const std::vector<std::filesystem::path>& a_selectedFilePathList, AssetBrowserEditorWindow& a_editorWindow) const
{
    // FileOperation::Deleteを直接よばす
    // 削除確認ダイアログをリクエストする
    // ユーザーがダイアログで削除を選んだ場合に実際の削除処理が行われる
    auto l_deleteConfirmPopup = a_editorWindow.GetREFDeleteConfirmPopup();

    l_deleteConfirmPopup.Request(a_selectedFilePathList, a_editorWindow);
}