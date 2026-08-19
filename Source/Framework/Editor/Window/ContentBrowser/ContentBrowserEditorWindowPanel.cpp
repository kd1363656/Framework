#include "ContentBrowserEditorWindowPanel.h"

void FWK::Editor::ContentBrowserEditorWindowPanel::DrawFolderTree(ContentBrowserEditorWindow& a_contentBrowserEditorWindow)
{
    // 左ペインのFolderTree専用のChildeWindowを作成する
    if (const ImVec2 l_folderTreePanelSize = { k_folderTreePanelWidth, k_fillRemainingSize };
        !ImGui::BeginChild(k_folderTreeChildLabel.data(), l_folderTreePanelSize, true))
    {
        ImGui::EndChild();

        return; 
    }

    // ContentBrowserのRootであるContent事態がそんz寧しなければfolderTreeは描画できない
    if (std::error_code l_errorCode = {};
        std::filesystem::exists(Constant::k_contentRootFolderPath, l_errorCode) &&
        !l_errorCode)
    {
        l_errorCode.clear();

        // Contentが存在していても
        // Fileだった場合はFolderTreeのRootとして使用できない
        if (std::filesystem::is_directory(Constant::k_contentRootFolderPath, l_errorCode) &&
            !l_errorCode)
        {
            // RootであるContentから
            // 再帰的なFolderTree描画を開始する
            DrawFolderTreeNode(Constant::k_contentRootFolderPath, a_contentBrowserEditorWindow);
        }
    }

    // BeginChild()がfalseを返した場合でも
    // EndChild(9は必ず呼ぶ必要がある
    ImGui::EndChild();
}

void FWK::Editor::ContentBrowserEditorWindowPanel::DrawCurrentFolder(ContentBrowserEditorWindow& a_contentBrowserEditorWindow)
{

}

void FWK::Editor::ContentBrowserEditorWindowPanel::DrawFolderTreeNode(const std::filesystem::path& a_folderPath, ContentBrowserEditorWindow& a_contentBrowserEditorWindow)
{
    std::error_code l_errorCode = {};

    // FolderTreeにはdirectoryだけを表示する
    // Fileや無効なPathが渡された場合は描画しない
    if (!std::filesystem::is_directory(a_folderPath, l_errorCode) ||
        l_errorCode)
    {
        return;
    }

    const auto& l_fileSystem     = a_contentBrowserEditorWindow.GetREFFileSystem();
    const bool  l_hasChildFolder = l_fileSystem.HasChildFolder                  (a_folderPath);

    auto l_treeNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow |
                           ImGuiTreeNodeFlags_SpanAvailWidth;

    const auto& l_currentFolderPath = a_contentBrowserEditorWindow.GetREFCurrentFolderPath();

    // 右ペインで現在開いているFolderを
    // 左FolderTreeでも選択状態として表示する
    if (l_currentFolderPath == a_folderPath)
    {
        l_treeNodeFlags |= ImGuiTreeNodeFlags_Selected;
    }

    // 子Folderが存在しない場合はLeafとして扱う
    if (!l_hasChildFolder)
    {
        l_treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;
        l_treeNodeFlags |= ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }

    // ContentBrowserを初めて聞いた時だけ
    // RootであるContentを展開しておく
    if (a_folderPath == Constant::k_contentRootFolderPath)
    {
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    }

    // 同名Folderが別階層に存在しても
    // ImGuiIDが衝突しないようにPath全体をIDとして使用する
    const auto& l_folderPathString = a_folderPath.generic_string();
          auto  l_folderName       = a_folderPath.filename      ().string();

    if (l_folderName.empty())
    {
        l_folderName = l_folderPathString;
    }

    const bool l_isNodeOpen = ImGui::TreeNodeEx(l_folderPathString.c_str(),
                                                l_treeNodeFlags,
                                                "%s %s", 
                                                Constant::k_fontAwesomeFolderIcon.data(), 
                                                l_folderName.c_str());

    // この後DragDropTarget処理を行うので
    // TreeNode自身の状態は先の保存しておく
    // ImGui::IsItem~系は直前Itemを対象にするため
    const bool l_isNodeClicked     = ImGui::IsItemClicked    (ImGuiMouseButton_Left);
    const bool l_isNodeToggledOpen = ImGui::IsItemToggledOpen();

    // 左FolderTree上のFolderも
    // GameObjectのPrefab保存先として使用できるようにする
    DrawGameObjectPrefabDragDropTarget(a_folderPath, a_contentBrowserEditorWindow);

    // Arrow操作ではCurrentFolderを変更しない
    // Folder行そのものをClickした場合だけ右ペインを変更する
    if (l_isNodeClicked &&
        !l_isNodeToggledOpen)
    {
        a_contentBrowserEditorWindow.ApplyCurrentFolderPath(a_folderPath);
    }

    // 閉じているFolderやLeafなら、
    // これ以上子Folderを捜査する必要はない
    if (!l_isNodeOpen ||
        !l_hasChildFolder)
    {
        return;
    }

    std::filesystem::directory_iterator l_directoryITR    = { a_folderPath, l_errorCode };
    std::filesystem::directory_iterator l_endDirectoryITR = {};

    if (l_errorCode)
    {
        ImGui::TreePop();

        return;
    }

    while (l_directoryITR != l_endDirectoryITR)
    {
        std::error_code l_entryErrorCode = {};

        // 左ペインにはFileを表示しない
        // Folderだけを再帰的に描画する
        if (l_directoryITR->is_directory(l_entryErrorCode) &&
            !l_entryErrorCode)
        {
            DrawFolderTreeNode(l_directoryITR->path(), a_contentBrowserEditorWindow);
        }

        l_directoryITR.increment(l_errorCode);

        if (l_errorCode) { break; }
    }

    ImGui::TreePop();
}

bool FWK::Editor::ContentBrowserEditorWindowPanel::DrawFolderEntry(const Struct::ContentBrowserEntryData& a_entryData, ContentBrowserEditorWindow& a_contentBrowserEditorWindow)
{
    return false;
}

void FWK::Editor::ContentBrowserEditorWindowPanel::DrawGameObjectPrefabDragDropTarget(const std::filesystem::path& a_folderPath, ContentBrowserEditorWindow& a_contentBrowserEditorWindow) const
{
    auto& l_dragDropPayloadStorage = Utility::IMGUIDragDropPayloadStorage::GetInstance();

    std::weak_ptr<GameObject>l_gameObject = {};

    // WorldOutlinerから送られている
    // GameObjectのDrag&DropPayloadを受け取る
    if (!l_dragDropPayloadStorage.DragDropTarget(Constant::k_gameObjectDragDropPayloadLabel, l_gameObject)) { return; }

    // PanelはPrefabそのものを作成しない
    a_contentBrowserEditorWindow.CreatePrefabFromGameObject(l_gameObject, a_folderPath);
}

void FWK::Editor::ContentBrowserEditorWindowPanel::DrawCurrentFolderContextMenu(ContentBrowserEditorWindow& a_contentBrowserEditorWindow)
{

}

void FWK::Editor::ContentBrowserEditorWindowPanel::DrawFolderEntryContextMenu(const Struct::ContentBrowserEntryData& a_entryData, ContentBrowserEditorWindow& a_contentBrowserEditorWindow)
{
    if (!a_entryData.m_isSelectable) { return; }

          auto& l_entryController = a_contentBrowserEditorWindow.GetMutableREFEntryController();
    const auto& l_entryPath       = a_entryData.m_entryPath;

    // 削除ContextMenuはContentBrowserWindow内
    // 現在のEntry上で右クリックした場合だけ
    if (ImGui::IsWindowHovered() &&
        ImGui::IsItemHovered()   &&
        ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    {
        // 右クリックしたEntryがまだSelectionへ含まれていない場合は
        // 通常Clickと同じようにそのEntryだけを選択する
        // A.json -> 選択中
        // B.json -> 選択中
        // C.json -> 未選択 <- 右クリック
        // の場合はA/Bの選択を解除しCだけを選択し、Cを削除対象にする
        if (!l_entryController.ContainsSelectedEntry(l_entryPath))
        {
            l_entryController.SelectSingleEntry(l_entryPath);
        }

        // 既に選択中のEntryを右クリックした場合は
        // 現在の複数Selectionをそのまま維持する
        // A.json -> 選択中
        // B.json -> 選択中
        // C.json -> 選択中 <- 右クリック
        // ならA/B/C全てを削除対象として維持する
        ImGui::OpenPopup(k_folderEntryContextMenuLabel.data());

        // Popupを開いた後はWindowHoverを要求しない
        // Popup事態は別ImGuiWindowなので
        // ここでもIsWindowHovered()を条件にすると
        // Popup上へMouseを移動した瞬間に操作できなくなる
        if (!ImGui::BeginPopup(k_folderEntryContextMenuLabel.data())) { return; }

        if (ImGui::MenuItem(k_deleteEntryMenuItemLabel.data()))
        {
            // Entry一覧描画中に実際の削除を行わず
            // ContentBrowserEditorWindowへ削除要求だけ通知する
            // 実際には現在選択されているFile / Folder全てが削除対象になる
            a_contentBrowserEditorWindow.RequestSelectedEntryDelete();
        }

        ImGui::EndPopup();
    }
}

void FWK::Editor::ContentBrowserEditorWindowPanel::ApplyEntrySelectionShortcut(ContentBrowserEditorWindow& a_contentBrowserEditorWindow) const
{
    const auto& l_imGuiIO = ImGui::GetIO();

    // InputTextは編集中の場合
    // Ctrl + Aは文字列の全選択として使用する
    // ContentBrowserのEntry全選択として奪わない
    if (l_imGuiIO.WantTextInput) { return; }

    // ContentBrowserまたはそのChildWindowに
    // Focusがある倍だけCtrl + Aを受け付ける
    if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) ||
                                !l_imGuiIO.KeyCtrl                     ||
                                !ImGui::IsKeyPressed(ImGuiKey_A, false))
    {
        return;
    }

    auto& l_entryController = a_contentBrowserEditorWindow.GetMutableREFEntryController();

    l_entryController.SelectAllEntries();
}

std::string_view FWK::Editor::ContentBrowserEditorWindowPanel::FetchVALFolderEntryIcon(const std::filesystem::path& a_entryPath, const bool a_isFolder) const
{
    // Folderの場合は拡張子を見る必要がない
    if (a_isFolder) { return Constant::k_fontAwesomeFolderIcon; }

    const auto& l_extension = a_entryPath.extension();

    // FBXは3Dモデルを表すCubeIconを使用
    if (l_extension == Constant::k_lowerFBXExtension) { return Constant::k_fontAwesomeCubeIcon; }

    // PNGはImageIconを使用する
    if (l_extension == Constant::k_lowerPNGExtension) { return Constant::k_fontAwesomeImageIcon; }

    // Jsonを含む、それ以外のFileは
    // 八病FileIconを使用する
    return Constant::k_fontAwesomeFileIcon;
}