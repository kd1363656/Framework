#include "AssetBrowserEditorWindowShortcutHandler.h"

void FWK::Editor::AssetBrowserEditorWindowShortcutHandler::Handle(const std::vector<std::filesystem::path>& a_selectedFilePathList, const std::filesystem::path& a_targetFilePath, AssetBrowserEditorWindow& a_editorWindow) const
{
    // アクティブPane無効の場合は何もしない
    if (const auto l_activePane = a_editorWindow.GetVALActivePane();
        l_activePane == Enum::AssetBrowserActivePaneType::Invalid)
    {
        return; 
    }

    // ImGuiの入力状態を取得
    // ImGui::GetIO()は現在のフレームの入力状態を持つImGuiIO構造体を返す
    const auto& l_io = ImGui::GetIO();

    // 選択中のファイルがあるか
    const bool l_hasSelection = !a_selectedFilePathList.empty();

    // 単一選択かどうか(名前変更は単一選択時のみ有効)
    const bool l_isSingleSelection = (a_selectedFilePathList.size() == Constant::k_editorSelectedFolderSingleSize);

    // 複数選択中かどうか
    // 複数選択中は名前変更・新規フォルダ作成を無効にする
    const bool l_isMultiSelection = a_selectedFilePathList.size() > Constant::k_editorSelectedFolderSingleSize;

    // クリップボードが空でないか(貼り付けの判定に使用)
          auto& l_clipboard     = a_editorWindow.GetMutableREFClipboard    ();
          auto& l_renameState   = a_editorWindow.GetMutableREFRenameState  ();
          auto& l_fileOperation = a_editorWindow.GetMutableREFFileOperation();
    const bool  l_canPaste      = !l_clipboard.IsEmpty                     ();

    // 対象フォルダがルートフォルダかどうか
    const bool l_isRootFolder = a_targetFilePath == Constant::k_assetRootFolderPath;

    // 選択中リストにルートフォルダが含まれているかどうか
    const bool l_containsRoot = std::find(a_selectedFilePathList.begin(), a_selectedFilePathList.end(), Constant::k_assetRootFolderPath) != a_selectedFilePathList.end();

    // 新規作成
    // Ctrl + Shift + N : 新規フォルダ作成
    // KeyCtrlとKeyShiftが両方trueで、Nキーが押された瞬間
    // ImGui::IsKeyPressed()は押された瞬間trueを返す
    if (l_io.KeyCtrl                    &&
        l_io.KeyShift                   &&
        ImGui::IsKeyPressed(ImGuiKey_N) &&
        !l_isMultiSelection)
    {
        HandleCreateFolder(a_targetFilePath, a_editorWindow);
    }

    // 操作
    // F2 : 名前変更
    // 単一選択時のみ有効
    // ImGuiKey_F2はF2キーを表す
    if (l_isSingleSelection              &&
        !l_isRootFolder                  &&
        ImGui::IsKeyPressed(ImGuiKey_F2) &&
        !l_isMultiSelection)
    {
        HandleRename(a_targetFilePath, l_renameState);
    }

    // Ctrl + C : コピー
    // 選択中のファイルがある場合のみ
    if (l_hasSelection  &&
        !l_containsRoot &&
        l_io.KeyCtrl    && 
        ImGui::IsKeyPressed(ImGuiKey_C))
    {
        HandleCopy(a_selectedFilePathList, l_fileOperation, l_clipboard);
    }

    // Ctrl + X : 切り取り
    // 選択中のファイルがある場合のみ
    if (l_hasSelection  &&
        !l_containsRoot &&
        l_io.KeyCtrl    &&
        ImGui::IsKeyPressed(ImGuiKey_X))
    {
        HandleCut(a_selectedFilePathList, l_fileOperation, l_clipboard);
    }

    // Ctrl + V : 貼り付け
    // クリップボードが空でない場合のみ
    if (l_canPaste   &&
        l_io.KeyCtrl &&
        ImGui::IsKeyPressed(ImGuiKey_V))
    {
        HandlePaste(a_targetFilePath, l_fileOperation, l_clipboard);
    }

    // Ctrl + D : 複製
    // 選択中のファイルがある場合のみ
    if (l_hasSelection  &&
        !l_containsRoot &&
        l_io.KeyCtrl    &&
        ImGui::IsKeyPressed(ImGuiKey_D))
    {
        HandleDuplicate(a_selectedFilePathList, l_fileOperation);
    }

    // Del : 削除
    // 選択中のファイルがある場合のみ
    // ImGuiKey_DeleteはDeleteキーを表す
    if (l_hasSelection  &&
        !l_containsRoot &&
        ImGui::IsKeyPressed(ImGuiKey_Delete))
    {
        HandleDelete(a_selectedFilePathList, a_editorWindow);
    }
}

void FWK::Editor::AssetBrowserEditorWindowShortcutHandler::HandleFolderPane(AssetBrowserEditorWindow& a_editorWindow)
{
    const auto& l_io         = ImGui::GetIO                          ();
          auto& l_folderPane = a_editorWindow.GetMutableREFFolderPane();

    // Ctrl + A : 現在フォルダツリーで表示中のフォルダをすべて選択する
    // ImGui::IsKeyPressed(ImGuiKey_A) : Aキーが押された瞬間か
    // FetchVALDisplayedFolderList()で表示中フォルダリストを取得し
    // SelectionState::SelectAllで全選択する
    // ルートフォルダ(Asset)も選択対象に含まれるが
    // 既存のHandle側でルートフォルダ含む場合は操作無効化されるため問題ない
    if (l_io.KeyCtrl &&
        ImGui::IsKeyPressed(ImGuiKey_A))
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
    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
    {
        l_folderPane.MoveSelectionUp(a_editorWindow, l_io.KeyShift || 
                                                     l_io.KeyCtrl);
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
    {
        l_folderPane.MoveSelectionDown(a_editorWindow, l_io.KeyShift ||
                                                       l_io.KeyCtrl);
    }

    // 右キー左キーによる操作の反映
    if (ImGui::IsKeyPressed(ImGuiKey_RightArrow))
    {
        l_folderPane.ForciblyFolderOpen(a_editorWindow);
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
    {
        l_folderPane.ForciblyFolderClose(a_editorWindow);
    }

    // エンターキーで開閉の切り替え
    if (ImGui::IsKeyPressed(ImGuiKey_Enter))
    {
        l_folderPane.ToggleCurrentFolderOpen(a_editorWindow);
    }
}

void FWK::Editor::AssetBrowserEditorWindowShortcutHandler::HandleCreateFolder(const std::filesystem::path& a_parentFolderPath, AssetBrowserEditorWindow& a_editorWindow) const
{
    const auto& l_assetCreator = a_editorWindow.GetREFAssetCreator     ();
          auto& l_folderPane   = a_editorWindow.GetMutableREFFolderPane();
          auto& l_renameState = a_editorWindow.GetMutableREFRenameState();

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
    // 選択状態になることでハイライト表示され
    // 次の操作(コピー/切り取り/複製等)の対象になる
    auto& l_selectionState = l_folderPane.GetMutableREFSelectionState();

    l_selectionState.SelectSingleFolder(l_result.m_createdFilePath, a_editorWindow);

    // 作成成功時、名前へ移行モードへ移行
    // PopupDrawer::StartRenameと同じ処理だが、
    // ShortcutHandlerはPopupDrawerに依存せずに独自に名前変更モードを起動する
    l_renameState.m_targetFilePath = l_result.m_createdFilePath;
    l_renameState.m_isActive       = true;

    // InputTextの初期値としてファイル名を設定
    const auto& l_stem = l_result.m_createdFilePath.stem().string();

    // バッファをゼロクリア
    l_renameState.m_inputBuffer.fill(Constant::k_nullCharacter);

    // ファイル名をバッファへコピー
    const auto l_copySize = std::min(l_stem.size(), l_renameState.m_inputBuffer.size() - Constant::k_inputBufferLastSizeOffsetForCopy);

    std::copy_n(l_stem.begin(), l_copySize, l_renameState.m_inputBuffer.begin());

    // 初回フォーカス制御用フラグをリセット
    l_renameState.m_isFocused = false;
}

void FWK::Editor::AssetBrowserEditorWindowShortcutHandler::HandleRename(const std::filesystem::path& a_targetFilePath, Struct::AssetBrowserEditorWindowRenameState& a_renameState) const
{
    // 名前変更対象のパスを設定
    a_renameState.m_targetFilePath = a_targetFilePath;

    // 名前変更モードをアクティブにする
    a_renameState.m_isActive = true;

    // InputTextの初期値としてファイル名を設定
    const auto& l_stem = a_targetFilePath.stem().string();

    // バッファをゼロクリア
    a_renameState.m_inputBuffer.fill(Constant::k_nullCharacter);

    // ファイル名をバッファへコピー
    const auto& l_copySize = std::min(l_stem.size(), a_renameState.m_inputBuffer.size() - Constant::k_inputBufferLastSizeOffsetForCopy);

    std::copy_n(l_stem.begin(), l_copySize, a_renameState.m_inputBuffer.begin());
}
void FWK::Editor::AssetBrowserEditorWindowShortcutHandler::HandleCopy(const std::vector<std::filesystem::path>& a_selectedFilePathList, AssetBrowserEditorWindowFileOperation& a_fileOperation, AssetBrowserEditorWindowClipboard& a_clipboard) const
{
    // FileOperation::Copyで選択中のファイルをクリップボードへコピー
    // コピー元ファイルは削除されない
    a_fileOperation.Copy(a_selectedFilePathList, a_clipboard);
}
void FWK::Editor::AssetBrowserEditorWindowShortcutHandler::HandleCut(const std::vector<std::filesystem::path>& a_selectedFilePathList, AssetBrowserEditorWindowFileOperation& a_fileOperation, AssetBrowserEditorWindowClipboard& a_clipboard) const
{
    // FileOperation::Cutで選択中のファイルをクリップボードへ切り取り
    // 貼り付け時に元ファイルが削除される
    a_fileOperation.Cut(a_selectedFilePathList, a_clipboard);
}
void FWK::Editor::AssetBrowserEditorWindowShortcutHandler::HandlePaste(const std::filesystem::path& a_targetFolderPath, AssetBrowserEditorWindowFileOperation& a_fileOperation, AssetBrowserEditorWindowClipboard& a_clipboard) const
{
    // FileOperation::Pasteでクリップボードのファイルを現在フォルダへ張り付け
    // Cutの場合は元ファイルを削除、Copyの場合は複製
    a_fileOperation.Paste(a_targetFolderPath, a_clipboard);
}
void FWK::Editor::AssetBrowserEditorWindowShortcutHandler::HandleDuplicate(const std::vector<std::filesystem::path>& a_selectedFilePathList, AssetBrowserEditorWindowFileOperation& a_fileOperation) const
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