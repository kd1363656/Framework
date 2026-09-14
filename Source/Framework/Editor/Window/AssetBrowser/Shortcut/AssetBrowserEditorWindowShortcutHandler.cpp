#include "AssetBrowserEditorWindowShortcutHandler.h"

void FWK::Editor::AssetBrowserEditorWindowShortcutHandler::Handle(const std::vector<std::filesystem::path>&          a_selectedFilePathList, 
                                                                  const std::filesystem::path&                       a_parentFolderPath,
                                                                  const std::filesystem::path&                       a_targetFilePath, 
                                                                  const Enum::AssetBrowserActivePaneType             a_activePane, 
                                                                  const AssetBrowserEditorWindowAssetCreator&        a_assetCreator,
                                                                        AssetBrowserEditorWindowFileOperation&       a_fileOperation, 
                                                                        AssetBrowserEditorWindowClipboard&           a_clipboard,
                                                                        Struct::AssetBrowserEditorWindowRenameState& a_renameState) const
{
    // アクティブPane無効の場合は何もしない
    if (a_activePane == Enum::AssetBrowserActivePaneType::Invalid) { return; }

    // ImGuiの入力状態を取得
    // ImGui::GetIO()は現在のフレームの入力状態を持つImGuiIO構造体を返す
    const auto& l_io = ImGui::GetIO();

    // 選択中のファイルがあるか
    const bool l_hasSelection = !a_selectedFilePathList.empty();

    // 単一選択かどうか(名前変更は単一選択時のみ有効)
    const bool l_isSingleSelection = (a_selectedFilePathList.size() == Constant::k_editorSelectedFolderSingleSize);

    // クリップボードが空でないか(貼り付けの判定に使用)
    const bool l_canPaste = !a_clipboard.IsEmpty();

    // 新規作成
    // Ctrl + Shift + N : 新規フォルダ作成
    // KeyCtrlとKeyShiftが両方trueで、Nキーが押された瞬間
    // ImGui::IsKeyPressed()は押された瞬間trueを返す
    if (l_io.KeyCtrl  &&
        l_io.KeyShift &&
        ImGui::IsKeyPressed(ImGuiKey_N))
    {
        HandleCreateFolder(a_parentFolderPath, a_assetCreator, a_renameState);
    }

    // 操作
    // F2 : 名前変更
    // 単一選択時のみ有効
    // ImGuiKey_F2はF2キーを表す
    if (l_isSingleSelection &&
        ImGui::IsKeyPressed(ImGuiKey_F2))
    {
        HandleRename(a_targetFilePath, a_renameState);
    }

    // Ctrl + C : コピー
    // 選択中のファイルがある場合のみ
    if (l_hasSelection &&
        l_io.KeyCtrl   && 
        ImGui::IsKeyPressed(ImGuiKey_C))
    {
        HandleCopy(a_selectedFilePathList, a_fileOperation, a_clipboard);
    }

    // Ctrl + X : 切り取り
    // 選択中のファイルがある場合のみ
    if (l_hasSelection &&
        l_io.KeyCtrl   &&
        ImGui::IsKeyPressed(ImGuiKey_X))
    {
        HandleCut(a_selectedFilePathList, a_fileOperation, a_clipboard);
    }

    // Ctrl + V : 貼り付け
    // クリップボードが空でない場合のみ
    if (l_canPaste   &&
        l_io.KeyCtrl &&
        ImGui::IsKeyPressed(ImGuiKey_V))
    {
        HandlePaste(a_parentFolderPath, a_fileOperation, a_clipboard);
    }

    // Ctrl + D : 複製
    // 選択中のファイルがある場合のみ
    if (l_hasSelection &&
        l_io.KeyCtrl   &&
        ImGui::IsKeyPressed(ImGuiKey_D))
    {
        HandleDuplicate(a_selectedFilePathList, a_fileOperation);
    }

    // Del : 削除
    // 選択中のファイルがある場合のみ
    // ImGuiKey_DeleteはDeleteキーを表す
    if (l_hasSelection &&
        ImGui::IsKeyPressed(ImGuiKey_Delete))
    {
        HandleDelete(a_selectedFilePathList, a_fileOperation);
    }
}

void FWK::Editor::AssetBrowserEditorWindowShortcutHandler::HandleCreateFolder(const std::filesystem::path& a_parentFolderPath, const AssetBrowserEditorWindowAssetCreator& a_assetCreator, Struct::AssetBrowserEditorWindowRenameState& a_renameState) const
{
    // AssetCreator::CreateFolderでフォルダを作成
    // 戻り値がCreationResultに作成パスと成否が入っている
    const auto& l_result = a_assetCreator.CreateFolder(a_parentFolderPath);

    if (!l_result.m_isSuccess) { return; }

    // 作成成功時、名前へ移行モードへ移行
    // PopupDrawer::StartRenameと同じ処理だが、
    // ShortcutHandlerはPopupDrawerに依存せずに独自に名前変更モードを起動する
    a_renameState.m_targetFilePath = l_result.m_createdFilePath;
    a_renameState.m_isActive       = true;

    // InputTextの初期値としてファイル名を設定
    const auto& l_stem = l_result.m_createdFilePath.stem().string();

    // バッファをゼロクリア
    a_renameState.m_inputBuffer.fill(Constant::k_nullCharacter);

    // ファイル名をバッファへコピー
    const auto l_copySize = std::min(l_stem.size(), a_renameState.m_inputBuffer.size() - Constant::k_inputBufferLastSizeOffsetForCopy);

    std::copy_n(l_stem.begin(), l_copySize, a_renameState.m_inputBuffer.begin());

    // 初回フォーカス制御用フラグをリセット
    a_renameState.m_isFocused = false;
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
void FWK::Editor::AssetBrowserEditorWindowShortcutHandler::HandlePaste(const std::filesystem::path& a_parentFolderPath, AssetBrowserEditorWindowFileOperation& a_fileOperation, AssetBrowserEditorWindowClipboard& a_clipboard) const
{
    // FileOperation::Pasteでクリップボードのファイルを現在フォルダへ張り付け
    // Cutの場合は元ファイルを削除、Copyの場合は複製
    a_fileOperation.Paste(a_parentFolderPath, a_clipboard);
}
void FWK::Editor::AssetBrowserEditorWindowShortcutHandler::HandleDuplicate(const std::vector<std::filesystem::path>& a_selectedFilePathList, AssetBrowserEditorWindowFileOperation& a_fileOperation) const
{
    // FileOperation::Duplicateで選択中のファイルを複製
    // 同名の場合は自動で番号付与される(Player -> Player1)
    a_fileOperation.Duplicate(a_selectedFilePathList);
}
void FWK::Editor::AssetBrowserEditorWindowShortcutHandler::HandleDelete(const std::vector<std::filesystem::path>& a_selectedFilePathList, AssetBrowserEditorWindowFileOperation& a_fileOperation) const
{
    // FileOperation::Deleteで選択中のファイルを削除
    // std::filesystem::removeで物理削除される
    a_fileOperation.Delete(a_selectedFilePathList);
}