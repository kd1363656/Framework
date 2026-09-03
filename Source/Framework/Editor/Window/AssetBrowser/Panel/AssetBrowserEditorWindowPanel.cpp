#include "AssetBrowserEditorWindowPanel.h"

void FWK::Editor::AssetBrowserEditorWindowPanel::DrawFolderTree(AssetBrowserEditorWindow& a_assetBrowserEditorWindow)
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
            DrawFolderTreeNode(Constant::k_contentRootFolderPath, a_assetBrowserEditorWindow);
        }
    }

    // BeginChild()がfalseを返した場合でも
    // EndChild(9は必ず呼ぶ必要がある
    ImGui::EndChild();
}
void FWK::Editor::AssetBrowserEditorWindowPanel::DrawCurrentFolder(AssetBrowserEditorWindow& a_assetBrowserEditorWindow)
{
    // k_fillRemainingSizeは0.0fだがImGuiでは{ 0.0F, 0.0F }で親ウィンドウの残っているすべての領域を使う
    // 右ペインすべての領域を使うということ
    if (const ImVec2& l_currentFolderPanelSize = { k_fillRemainingSize, k_fillRemainingSize };
        !ImGui::BeginChild(k_currentFolderChildLabel.data(), l_currentFolderPanelSize, true))
    {
        ImGui::EndChild();

        return;
    }

    std::error_code l_errorCode = {};

    // Explorer等からCurrentFolderを削除された場合は
    // ContentRootへ戻す
    if (const auto& l_currentFolderPath = a_assetBrowserEditorWindow.GetREFCurrentFolderPath();
        !std::filesystem::is_directory(l_currentFolderPath, l_errorCode) ||
        l_errorCode)
    {
        a_assetBrowserEditorWindow.ApplyCurrentFolderPath(Constant::k_contentRootFolderPath);

        l_errorCode.clear();
    }

    // ContentRoot自体まで存在しない場合は
    // Entryを描画できない
    if (const auto& l_validCurrentFolderPath = a_assetBrowserEditorWindow.GetREFCurrentFolderPath();
        !std::filesystem::is_directory(l_validCurrentFolderPath, l_errorCode) ||
        l_errorCode)
    {
        ImGui::EndChild();

        return;
    }

    const auto& l_entryController = a_assetBrowserEditorWindow.GetREFEntryController();

    // Dirtyの時だけFilesystemを走査する
    if (l_entryController.GetVALIsCurrentFolderEntryListDirty())
    {
        a_assetBrowserEditorWindow.RefreshCurrentFolderEntries();
    }

    // Ctrl + Aで現在表示しているEntryをすべて選択する
    ApplyEntrySelectionShortcut(a_assetBrowserEditorWindow);

    // 選択中Folderが1つだけならEnterキーでそのEnterキーでそのFolderを開く
    ApplySelectedFolderOpenShortcut(a_assetBrowserEditorWindow);

    // Deleteキーで現在選択されているEntryを削除要求する
    // 複数選択・全選択も既存の削除処理がまとめて処理する
    ApplySelectedEntryDeleteShortcut(a_assetBrowserEditorWindow);

    // フォルダ作成ショートカットの受付
    ApplyFolderCreateShortcut(a_assetBrowserEditorWindow);

    DrawFolderCreateEntry(a_assetBrowserEditorWindow);

    const float l_availableWidth   = ImGui::GetContentRegionAvail().x;
    const float l_itemSpacing      = ImGui::GetStyle             ().ItemSpacing.x;
    const float l_folderEntryPitch = k_folderEntryWidth + l_itemSpacing;

    // 何列表示できるのかを計算Cardの横幅 + スペース幅で現在の右ペインで何個横に並べれるかを計算している
    const auto l_calculatedColumnCount = static_cast<std::uint32_t>((l_availableWidth + l_itemSpacing) / l_folderEntryPitch);
    const auto l_columnCount           = std::max                  (k_minFolderEntryColumnCount, l_calculatedColumnCount);
          auto l_currentColumn         = k_initialFolderEntryColumnCount;

    // FolderをDoubleClickした場合、
    // Entry一覧走査中にはCurrentFolderを変更しない
    std::filesystem::path l_requestedDirectoryPath = {};

    // Entry一覧のvectorはEntryControllerが唯一所有する
    // const参照なのでコピーは発生しない
    const auto& l_currentFolderEntryDataList = l_entryController.GetREFCurrentFolderEntryDataList();

    for (const auto& l_entryData : l_currentFolderEntryDataList)
    {
        if (DrawFolderEntry(l_entryData, a_assetBrowserEditorWindow))
        {
            l_requestedDirectoryPath = l_entryData.m_entryPath;
        }

        ++l_currentColumn;

        if (l_currentColumn < l_columnCount)
        {
            ImGui::SameLine();
        }
        else
        {
            l_currentColumn = k_initialFolderEntryColumnCount;
        }
    }

    // Entry以外の空白ContextMenuを処理する
    DrawCurrentFolderContextMenu(a_assetBrowserEditorWindow);

    // Entry一覧の走査が完全に終了してから
    // DoubleClickされたFolderへ移動する
    if (!l_requestedDirectoryPath.empty())
    {
        a_assetBrowserEditorWindow.ApplyCurrentFolderPath(l_requestedDirectoryPath);
    }

    ImGui::EndChild();

    // currentFolderChildWindow全体への
    // GameObjectDropも受け取る
    DrawGameObjectPrefabDragDropTarget(a_assetBrowserEditorWindow.GetREFCurrentFolderPath(), a_assetBrowserEditorWindow);

    // シーンのDropを受け取る
    DrawSceneDragDropTarget(a_assetBrowserEditorWindow.GetREFCurrentFolderPath(), a_assetBrowserEditorWindow);
}

void FWK::Editor::AssetBrowserEditorWindowPanel::DrawFolderTreeNode(const std::filesystem::path& a_folderPath, AssetBrowserEditorWindow& a_assetBrowserEditorWindow)
{
        std::error_code l_errorCode = {};

    // FolderTreeにはdirectoryだけを表示する
    // Fileや無効なPathが渡された場合は描画しない
    if (!std::filesystem::is_directory(a_folderPath, l_errorCode) ||
        l_errorCode)
    {
        return;
    }

    const auto& l_fileSystem        = a_assetBrowserEditorWindow.GetREFFileSystem       ();
    const auto& l_currentFolderPath = a_assetBrowserEditorWindow.GetREFCurrentFolderPath();

    const bool l_hasChildFolder                            = l_fileSystem.HasChildFolder(a_folderPath);
    const bool l_isFolderTreeSynchronizedCurrentFolderPath = m_synchronizedCurrentFolderPath != l_currentFolderPath;
    
    auto l_treeNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow       |
                           ImGuiTreeNodeFlags_OpenOnDoubleClick |
                           ImGuiTreeNodeFlags_SpanAvailWidth;

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
    // 同名Folderが別階層に存在しても
    // ImGuiIDが衝突しないようにPath全体をIDとして使用する
    if (l_isFolderTreeSynchronizedCurrentFolderPath &&
        l_hasChildFolder                            &&
        ContainsCurrentFolderPath(a_folderPath, l_currentFolderPath))
    {
        ImGui::SetNextItemOpen(true, ImGuiCond_Always);
    }

    // 同じ名前のFolder別階層が存在していても
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
    DrawGameObjectPrefabDragDropTarget(a_folderPath, a_assetBrowserEditorWindow);
    DrawSceneDragDropTarget           (a_folderPath, a_assetBrowserEditorWindow);

    // このFolderが実際のCurrentFolderなら、
    // 左FolderTreeの動機が正常に完了したことになる
    if (l_isFolderTreeSynchronizedCurrentFolderPath &&
        l_currentFolderPath == a_folderPath)
    {
        // 不快階層へ右ペインから移動した場合、
        // Treeを展開しただけでは左ペインの画面外に
        // CurrentFolderが存在する可能性がある
        // 現在Folderが見える位置まで
        ImGui::SetScrollHereY(k_centeringRatio);

        m_synchronizedCurrentFolderPath = a_folderPath;
    }

    // _Arrow操作ではCurrentFolderを変更しない
    // Folderの行本体をClickした場合だけ、
    // 右ペインのCurrentFolderを変更する
    if (l_isNodeClicked &&
        !l_isNodeToggledOpen)
    {
        m_synchronizedCurrentFolderPath = a_folderPath;

        a_assetBrowserEditorWindow.ApplyCurrentFolderPath(a_folderPath);
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
        // 左ペインにはFileを表示しない
        // Folderだけを再帰的に描画する
        if (std::error_code l_entryErrorCode = {};
            l_directoryITR->is_directory(l_entryErrorCode) &&
            !l_entryErrorCode)
        {
            DrawFolderTreeNode(l_directoryITR->path(), a_assetBrowserEditorWindow);
        }

        l_directoryITR.increment(l_errorCode);

        if (l_errorCode) { break; }
    }

    ImGui::TreePop();
}
bool FWK::Editor::AssetBrowserEditorWindowPanel::DrawFolderEntry(const Struct::AssetBrowserEntryData& a_entryData, AssetBrowserEditorWindow& a_assetBrowserEditorWindow) const
{
    const auto& l_entryPath = a_entryData.m_entryPath;

    // ImGuiIDとして使用するため、
    // Entry名ではなくPath全体を文字列化する
    // Content/Character/Text.json
    // Content/Enemy/Text.json
    const auto& l_entryPathString = l_entryPath.generic_string();
    const auto& l_entryName       = l_entryPath.filename      ().string();
    const auto& l_icon            = FetchVALFolderEntryIcon   (l_entryPath, a_entryData.m_isFolder);

    ImGui::PushID(l_entryPathString.c_str());

    const ImVec2& l_entrySize = { k_folderEntryWidth, k_folderEntryHeight };

    // EntryCard全体を一つのItemとして登録する
    // InvisibleButton自体には見た目がないため、
    // 背景・Icon・Entry名は後ほどDrawListで描画する
    ImGui::InvisibleButton(k_folderEntryButtonLabel.data(), l_entrySize, ImGuiButtonFlags_MouseButtonLeft);

    // この後ContextMenuやDragDropTargetなど、
    // 別のImGui処理を呼び出す
    // ImGui::IsItemXXD()やGetItemRectXXX()は「直前のItem」を参照するため
    // Entry自身の情報はここですべて保存しておく
    const bool l_isHovered     = ImGui::IsItemHovered();
    const bool l_isLeftClicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
    const bool l_isDoubleClicked = a_entryData.m_isFolder &&
                                   l_isHovered            &&
                                   ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

    const auto& l_itemMIN = ImGui::GetItemRectMin();
    const auto& l_itemMAX = ImGui::GetItemRectMax();

    // FBX/ PNG/ JSONなどのFileを
    // AssetFilePath Payloadとして送信する
    // Folder移動は別のPayloadを使用する
    if (!a_entryData.m_isFolder)
    {
        auto& l_dragDropPayloadStorage = Utility::IMGUIDragDropPayloadStorage::GetInstance();

        // std::filesystem::pathをPayloadとして送る
        l_dragDropPayloadStorage.DragDropSource(Constant::k_assetFilePathDragAndDropPayloadLabel, l_entryPath);
    }

    // InvisibleButtonが現在のEntryなので
    // Entry右クリックContextMenuを処理する
    DrawFolderEntryContextMenu(a_entryData, a_assetBrowserEditorWindow);

    // Folderの場合だけ
    // OutlinerのGameObjectをDropできるPrefab保存先として扱う
    if (a_entryData.m_isFolder)
    {
        DrawGameObjectPrefabDragDropTarget(l_entryPath, a_assetBrowserEditorWindow);
        DrawSceneDragDropTarget           (l_entryPath, a_assetBrowserEditorWindow);
    }

    auto& l_entryController = a_assetBrowserEditorWindow.GetMutableREFEntryController();

    if (l_isLeftClicked &&
        a_entryData.m_isSelectable)
    {
        const auto& l_imGuiIO = ImGui::GetIO();

        // Shiftを先に判定する
        // Ctrl + Shiftが同時に押されている場合は
        // Shiftによる範囲選択を優先する
        if (l_imGuiIO.KeyShift)
        {
            l_entryController.SelectRangeEntry(l_entryPath);
        }
        else if (l_imGuiIO.KeyCtrl)
        {
            // 選択中Entryなら選択解除、
            // 未選択Entryなら現在Selectionへ追加する
            l_entryController.ToggleEntrySelection(l_entryPath);
        }
        else
        {
            // Modifierなしの通常Clickでは
            // このEntryだけ選択する
            l_entryController.SelectSingleEntry(l_entryPath);
        }
    }
    
    const bool l_isSelected = l_entryController.ContainsSelectedEntry(l_entryPath);

    auto* l_drawList = ImGui::GetWindowDrawList();

    if (!l_drawList) 
    {
        ImGui::PopID();

        return false; 
    }

    if (l_isSelected)
    {
        const auto& l_backgroundColor = ImGui::GetColorU32(l_isHovered ? ImGuiCol_HeaderActive : ImGuiCol_Header);

        l_drawList->AddRectFilled(l_itemMIN,
                                  l_itemMAX,
                                  l_backgroundColor,
                                  k_folderEntryRounding);
    }
    else if (l_isHovered)
    {
        const auto& l_backgroundColor = ImGui::GetColorU32(ImGuiCol_HeaderHovered);

        l_drawList->AddRectFilled(l_itemMIN,
                                  l_itemMAX,
                                  l_backgroundColor,
                                  k_folderEntryRounding);
    }

    auto* l_font = ImGui::GetFont();

    if (!l_font)
    {
        ImGui::PopID();

        return false;
    }

    const auto& l_iconSize = l_font->CalcTextSizeA(k_folderEntryIconFontSize,
                                                   std::numeric_limits<float>::max(),
                                                   k_fillRemainingSize,
                                                   l_icon.data());

    const float   l_iconPositionX = l_itemMIN.x + (k_folderEntryWidth - l_iconSize.x) * k_centeringRatio;
    const float   l_iconPositionY = l_itemMIN.y + k_folderEntryIconTopPadding;
    const ImVec2& l_iconPosition  = { l_iconPositionX, l_iconPositionY };

    const auto& l_textColor = ImGui::GetColorU32(ImGuiCol_Text);

    // FontAwesomeGlyphを通常文字より大きく描画する
    l_drawList->AddText(ImGui::GetFont(),
                        k_folderEntryIconFontSize,
                        l_iconPosition,
                        l_textColor,
                        l_icon.data());

    // 実際のFile/Folder名は変更しない
    // ContentBrowser上の表示文字列だけ必要に応じて短縮する
    // hogeeeeeeeeeがhogeeeeee...になる
    std::string l_displayEntryName = l_entryName;

    if (l_displayEntryName.size() >= k_folderEntryNameDisplayCharacterCount)
    {
        l_displayEntryName = l_displayEntryName.substr(k_folderEntryNameStartIndex, k_folderEntryNameDisplayCharacterCount);

        l_displayEntryName += k_folderEntryNameEllipsis;
    }

    const ImVec2& l_entryNameSize =  ImGui::CalcTextSize(l_displayEntryName.c_str());

    // Entry名をCardの横中央へ配置する
    const float l_textPositionX = l_itemMIN.x + (k_folderEntryWidth        - l_entryNameSize.x) * k_centeringRatio;
    const float l_textPositionY = l_itemMAX.y - ImGui::GetTextLineHeight() - k_folderEntryTextBottomPadding;

    const ImVec2& l_textPosition = { l_textPositionX,  l_textPositionY };

    l_drawList->AddText(l_textPosition, l_textColor, l_displayEntryName.c_str());

    ImGui::PopID();

    // FolderがDoubleClickされた場合だけtrueを返す
    // 実際にCurrentFolder変更は
    // Entry一覧Vectorの走査が完了してから
    // DrawCurrentFolder()側で行う
    return l_isDoubleClicked;
}
void FWK::Editor::AssetBrowserEditorWindowPanel::DrawGameObjectPrefabDragDropTarget(const std::filesystem::path& a_folderPath, AssetBrowserEditorWindow& a_assetBrowserEditorWindow) const
{
    auto& l_dragDropPayloadStorage = Utility::IMGUIDragDropPayloadStorage::GetInstance();

    std::weak_ptr<GameObject>l_gameObject = {};

    // WorldOutlinerから送られている
    // GameObjectのDrag&DropPayloadを受け取る
    if (!l_dragDropPayloadStorage.DragDropTarget(Constant::k_gameObjectDragDropPayloadLabel, l_gameObject)) { return; }

    // PanelはPrefabそのものを作成しない
    a_assetBrowserEditorWindow.CreatePrefabFromGameObject(l_gameObject, a_folderPath);
}
void FWK::Editor::AssetBrowserEditorWindowPanel::DrawCurrentFolderContextMenu(AssetBrowserEditorWindow& a_assetBrowserEditorWindow)
{
    const bool l_isCurrentFolderWindowHovered = ImGui::IsWindowHovered();
    const bool l_isAnyItemHovered             = ImGui::IsAnyItemHovered();

    auto& l_entryController = a_assetBrowserEditorWindow.GetMutableREFEntryController();

    // currentFolderの空白部分を左クリックした場合だけ
    // Entryの選択状態をすべて解除する
    if (l_isCurrentFolderWindowHovered &&
        !l_isAnyItemHovered            &&
        ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        l_entryController.ClearSelectedEntries();
    }

    // 空白部分を右クリックした場合は
    // 選択状態を維持したままFolder追加Menuを開く
    if (l_isCurrentFolderWindowHovered &&
        !l_isAnyItemHovered            &&
        ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    {
        ImGui::OpenPopup(k_currentFolderContextMenuLabel.data());
    }

    // Popupを開いた後は元WindowのHoverを要求しない
    if (!ImGui::BeginPopup(k_currentFolderContextMenuLabel.data())) { return; }

    if (ImGui::MenuItem(k_addFolderMenuItemLabel.data()))
    {
        a_assetBrowserEditorWindow.RequestFolderCreate(a_assetBrowserEditorWindow.GetREFCurrentFolderPath());
    }

    ImGui::EndPopup();
}
void FWK::Editor::AssetBrowserEditorWindowPanel::DrawSceneDragDropTarget(const std::filesystem::path& a_folderPath, AssetBrowserEditorWindow& a_assetBrowserEditorWindow) const
{
    auto& l_dragDropPayloadStorage = Utility::IMGUIDragDropPayloadStorage::GetInstance();

    std::weak_ptr<Scene> l_scene = {};

    // WorldOutlinerのSceneNodeから送信された
    // SceneDragDropPayloadを受け取る
    if (!l_dragDropPayloadStorage.DragDropTarget(Constant::k_sceneDragDropPayloadLabel, l_scene)) { return; }

    // Panel自身はSceneJsonを生成しない
    // 保存処理はContentBrowserEditorWindow側へ渡す
    a_assetBrowserEditorWindow.CreateSceneFromScene(l_scene, a_folderPath);
}
void FWK::Editor::AssetBrowserEditorWindowPanel::DrawFolderEntryContextMenu(const Struct::AssetBrowserEntryData& a_entryData, AssetBrowserEditorWindow& a_assetBrowserEditorWindow) const
{
    if (!a_entryData.m_isSelectable) { return; }

          auto& l_entryController = a_assetBrowserEditorWindow.GetMutableREFEntryController();
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
    }

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
        a_assetBrowserEditorWindow.RequestSelectedEntryDelete();
    }

    ImGui::EndPopup();
}
void FWK::Editor::AssetBrowserEditorWindowPanel::DrawFolderCreateEntry(AssetBrowserEditorWindow& a_assetBrowserEditorWindow) const
{
    if (!a_assetBrowserEditorWindow.GetVALIsFolderCreateActive()) { return; }

    auto& l_folderCreateNameBuffer = a_assetBrowserEditorWindow.GetMutableREFolderCreateNameBuffer();

    if (a_assetBrowserEditorWindow.GetVALIsFolderCreateInputFocusRequested())
    {
        ImGui::SetKeyboardFocusHere();

        a_assetBrowserEditorWindow.SetFolderCreateInputFocusRequested(false);
    }

    const bool l_isEnterPressed = ImGui::InputText(k_folderCreateInputLabel.data(),
                                                   &l_folderCreateNameBuffer,
                                                   ImGuiInputTextFlags_EnterReturnsTrue);

    if (l_isEnterPressed)
    {
        a_assetBrowserEditorWindow.ConfirmFolderCreate();

        return;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Escape, false))
    {
        a_assetBrowserEditorWindow.CancelFolderCreate();
    }
}

void FWK::Editor::AssetBrowserEditorWindowPanel::ApplyEntrySelectionShortcut(AssetBrowserEditorWindow& a_assetBrowserEditorWindow) const
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

    auto& l_entryController = a_assetBrowserEditorWindow.GetMutableREFEntryController();

    l_entryController.SelectAllEntries();
}
void FWK::Editor::AssetBrowserEditorWindowPanel::ApplySelectedFolderOpenShortcut(AssetBrowserEditorWindow& a_assetBrowserEditorWindow) const
{
    const auto& l_imGuiIO = ImGui::GetIO();

    // Folder作成用InputTextを編集中の場合は、
    // EnterキーをFolder移動として使用しない
    if (a_assetBrowserEditorWindow.GetVALIsFolderCreateActive() ||
        l_imGuiIO.WantTextInput)
    {
        return;
    }

    // ContentBrowserまたはそのChildWindowにFocusがあるときだけ
    // Enterショートカットを有効にする
    if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) ||
        !ImGui::IsKeyPressed(ImGuiKey_Enter, false))
    {
        return; 
    }
    
    const auto& l_entryCoontroller   = a_assetBrowserEditorWindow.GetREFEntryController ();
    const auto& l_selectedFolderPath = l_entryCoontroller.FetchVALSingleSelectedFolderPath();

    if (l_selectedFolderPath.empty()) { return; }
    
    a_assetBrowserEditorWindow.ApplyCurrentFolderPath(l_selectedFolderPath);
}
void FWK::Editor::AssetBrowserEditorWindowPanel::ApplySelectedEntryDeleteShortcut(AssetBrowserEditorWindow& a_assetBrowserEditorWindow) const
{
    const auto& l_imGuiIO = ImGui::GetIO();

    // InputText入力中はショートカットを奪わない
    if (l_imGuiIO.WantTextInput ||
        !ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
    {
        return; 
    }

    if (!l_imGuiIO.KeyCtrl  ||
        !l_imGuiIO.KeyShift ||
        !ImGui::IsKeyPressed(ImGuiKey_N, false))
    {
        return;
    }

    const auto& l_entryController = a_assetBrowserEditorWindow.GetREFEntryController();

    // Folderが一つだけ選択されている場合は
    // そのFolder内を選択先にする
    const auto& l_selectedFolderPath = l_entryController.FetchVALSingleSelectedFolderPath();

    if (!l_selectedFolderPath.empty())
    {
        a_assetBrowserEditorWindow.RequestFolderCreate(l_selectedFolderPath);

        return;
    }

    // Ctrl + Shift + Nでは
    // 現在開いているFolderを作成先とする
    a_assetBrowserEditorWindow.RequestFolderCreate(a_contentBrowserEditorWindow.GetREFCurrentFolderPath());
}
void FWK::Editor::AssetBrowserEditorWindowPanel::ApplyFolderCreateShortcut(AssetBrowserEditorWindow& a_assetBrowserEditorWindow) const
{
    // Folder各入力中などは、
    // DeleteキーをInputText内の文字削除として使用する
    if (const auto& l_imGuiIO = ImGui::GetIO();
        a_assetBrowserEditorWindow.GetVALIsFolderCreateActive() ||
        l_imGuiIO.WantTextInput)
    {
        return;
    }

    // ContentBrowserまたはそのChildWindowにFocusがある時だけ
    // Deleteキーによる削除を有効にする
    if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) ||
        !ImGui::IsKeyPressed(ImGuiKey_Delete, false))
    {
        return; 
    }

    // Selectionが存在しない場合は削除要求出さない
    if (const auto& l_entryController = a_assetBrowserEditorWindow.GetREFEntryController();
        l_entryController.FetchVALSelectedEntryCount() == k_emptySelectionCount)
    {
        return; 
    }

    a_assetBrowserEditorWindow.RequestSelectedEntryDelete();
}

std::string_view FWK::Editor::AssetBrowserEditorWindowPanel::FetchVALFolderEntryIcon(const std::filesystem::path& a_entryPath, const bool a_isFolder) const
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

bool FWK::Editor::AssetBrowserEditorWindowPanel::ContainsCurrentFolderPath(const std::filesystem::path& a_folderPath, const std::filesystem::path& a_currentFolderPath) const
{
    auto l_folderPathITR        = a_folderPath.begin       ();
    auto l_currentFolderPathITR = a_currentFolderPath.begin();

    while (l_folderPathITR != a_folderPath.end())
    {
        // CurrentFolder側が先に終わった場合、
        // a_folderPathのほうが深い階層に存在する
        // つまりCurrentFolder自身でも祖先Folderでもない
        if (l_currentFolderPathITR == a_currentFolderPath.end()) { return false; }

        // Folder単位でも異なる部分があれば
        // CurerntFolderへ到達する経路上のFolderではない
        if (*l_folderPathITR != *l_currentFolderPathITR) { return false; }

        ++l_folderPathITR;
        ++l_currentFolderPathITR;
    }

    // a_folderPathのすべての構成要素が
    // CurrentFolderの先頭側と一致しということ、つまり
    // CurrentFolder自身、またはCurrentFolderの祖先Folderのどちらか
    return true;
}