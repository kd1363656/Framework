#include "AssetBrowserEditorWindowFolderPane.h"

void FWK::Editor::AssetBrowserEditorWindowFolderPane::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::Editor::AssetBrowserEditorWindowFolderPane::Draw(AssetBrowserEditorWindow& a_editorWindow)
{
    const auto& l_paneSplitter = a_editorWindow.GetREFPaneSplitter   ();
    const auto& l_popupDrawer  = a_editorWindow.GetREFPopupDrawer    ();
    const auto  l_paneWidth    = l_paneSplitter.GetVALPrimaryPaneSize();

    // ImGui::BeginChild()は
    // 現在のWindowの内部にもう一つの描画領域を作成するAPI
    if (const ImVec2 l_folderPaneSize = { l_paneWidth, Constant::k_imguiRemainingSize.y };
        !ImGui::BeginChild(k_childLabel.data(), l_folderPaneSize, true))
    {
        // BeginChild()もBegin()と同様
        // 呼びだした場合は必ずEndChild()と組み合わせる
        ImGui::EndChild();

        return;
    }

    // フォルダペイン上でクリックされた場合、アクティブPaneをFolderPaneにする
    // これによりショートカットキーがフォルダペインに送られる
    // ImGui::IsWindowHovered : このChildWindow上にマウスがあるか
    // ImGui::IsMouseClicked  : このフレームでクリックされたか
    // 左クリック・右クリックどちらも出アクティブPaneを切り替える
    if (ImGui::IsWindowHovered() &&
        (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)))
    {
        a_editorWindow.SetActivePane(Enum::AssetBrowserActivePaneType::FolderPane);
    }

    ImGui::TextUnformatted(k_paneTitleLabel.data());
    ImGui::Separator      ();

    // ルートフォルダから再帰的にツリーノードを描画
    // a_assetRootFolderPathがツリーのルート(Assetフォルダ)
    // DrawTreeNodeはa_folderPath自身を1ノード描画し
    // 開いている場合は子ディレクトリに対して再帰的にDrawTreeNodeを呼ぶ
    DrawTreeNode(Constant::k_assetRootFolderPath, a_editorWindow);

    // 空スペース右クリック
    // フォルダノード以外の空スペースを右クリックした場合
    // 選択中フォルダのコンテキストメニューを開く
    // 選択中フォルダがない場合はAssetルートを対象する
    // ImGui::IsWindowHovered  : マウスが子のChildWindow上にあるか
    // ImGui::IsAnyItemHovered : いずれかのアイテム上にマウスがあるか
    // ノード上で右クリックした場合はDrawTreeNode側でOpenPopupされるため
    // ここでは空スペースを場合のみポップアップを開く
    // ImGui::IsMouseClicked : このフレームで右クリックされたか
    if (ImGui::IsWindowHovered()   &&
        !ImGui::IsAnyItemHovered() &&
        ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    {
        // 空スペース用ポップアップ描画
        // 固定IDでよい(同時に一つしか開かないため)
        l_popupDrawer.BeginPopup(k_emptySpaceContextMenuOpenLabel);
    }

    // 対象フォルダ : 選択中フォルダ、なければAssetルート
    const auto& l_targetFolder = m_selectedFilePathList.empty() ? Constant::k_assetRootFolderPath : m_selectedFilePathList.back();

    // 空スペース用ポップアップ描画
    // PopupDrawer : Draw内部でBeginPopup / EndPopupを呼ぶ
    // ポップアップが開いていない場合はDraw内部でreturnする
    l_popupDrawer.Draw(m_selectedFilePathList,
                       l_targetFolder,
                       k_emptySpaceContextMenuOpenLabel,
                       Enum::AssetBrowserPopupContextType::FolderPane_OnFolder,
                       a_editorWindow);

    ImGui::EndChild();
}

void FWK::Editor::AssetBrowserEditorWindowFolderPane::MoveSelectionUp(const std::unordered_map<std::filesystem::path, std::vector<std::filesystem::path>>& a_folderHierarchyMap)
{
    // 表示中ノードリストを構築
    // 矢印キー押下時のみ構築するため毎フレームのオーバーヘッドなし
    // Assetルートから再帰的に開いているフォルダの子を収集
    std::vector<std::filesystem::path> l_displayedFolderList = {};

    // 現在フォルダツリーで開いている部分のみ収集
    BuildDisplayedFolderList(a_folderHierarchyMap, Constant::k_assetRootFolderPath, l_displayedFolderList);

    // ナビゲーションカーソル位置を決定
    // m_currentFolderPathをカーソルとして使う
    // 空の場合はリスト先頭
    const auto& l_cursorPath = m_currentFolderPath.empty() ? l_displayedFolderList.front() : m_currentFolderPath;

    // カーソル位置をリストから検索
    auto l_cursorITR = std::find(l_displayedFolderList.begin(), l_displayedFolderList.end(), l_cursorPath);

    // リストに現在選択中のパスが見つからない場合は
    // 先頭を現在選択中のパスとして扱う
    if (l_cursorITR == l_displayedFolderList.end())
    {
        SelectFolder(a_folderHierarchyMap, l_displayedFolderList.front());

        return;
    }

    // 既に先頭なら何もしない
    if (l_cursorITR == l_displayedFolderList.begin()) { return; }

    // 前のノードを取得
          auto  l_prevITR  = std::prev(l_cursorITR);
    const auto& l_prevPath = *l_prevITR;

    SelectFolder(a_folderHierarchyMap, l_prevPath);
}
void FWK::Editor::AssetBrowserEditorWindowFolderPane::MoveSelectionDown(const std::unordered_map<std::filesystem::path, std::vector<std::filesystem::path>>& a_folderHierarchyMap)
{
    // 表示中ノードリストを構築
    // 矢印キー押下時のみ構築するため毎フレームのオーバーヘッドなし
    // Assetルートから再帰的に開いているフォルダの子を収集
    std::vector<std::filesystem::path> l_displayedFolderList = {};

    // 現在フォルダツリーで開いている部分のみ収集
    BuildDisplayedFolderList(a_folderHierarchyMap, Constant::k_assetRootFolderPath, l_displayedFolderList);

    // ナビゲーションカーソル位置を決定
    // m_currentFolderPathをカーソルとして使う
    // 空の場合はリスト先頭
    const auto& l_cursorPath = m_currentFolderPath.empty() ? l_displayedFolderList.front() : m_currentFolderPath;

    // カーソル位置をリストから検索
    auto l_cursorITR = std::find(l_displayedFolderList.begin(), l_displayedFolderList.end(), l_cursorPath);

    // リストに現在選択中のパスが見つからない場合は
    // 先頭を現在選択中のパスとして扱う
    if (l_cursorITR == l_displayedFolderList.end())
    {
        SelectFolder(a_folderHierarchyMap, l_displayedFolderList.front());

        return;
    }

    // 次のノードを取得
    auto l_nextITR = std::next(l_cursorITR);

    // 既に末尾なら何もしない
    if (l_nextITR == l_displayedFolderList.end()) { return; }

    const auto& l_nextPath = *l_nextITR;

    SelectFolder(a_folderHierarchyMap, l_nextPath);
}
void FWK::Editor::AssetBrowserEditorWindowFolderPane::ForciblyFolderOpen()
{
    const auto& l_currentPath = m_selectedFilePathList.empty() ? m_currentFolderPath : m_selectedFilePathList.back();

    // 閉じている場合のみ展開する
    if (!IsFolderOpen(l_currentPath))
    {
        ToggleFolderOpen(l_currentPath);
    }
}
void FWK::Editor::AssetBrowserEditorWindowFolderPane::ForciblyFolderClose()
{
    const auto& l_currentPath = m_selectedFilePathList.empty() ? m_currentFolderPath : m_selectedFilePathList.back();

    // 閉じている場合のみ展開する
    if (IsFolderOpen(l_currentPath))
    {
        ToggleFolderOpen(l_currentPath);
    }
}

void FWK::Editor::AssetBrowserEditorWindowFolderPane::ToggleCurrentFolderOpen()
{
    if (m_currentFolderPath.empty()) { return; }

    ToggleFolderOpen(m_currentFolderPath);
}

void FWK::Editor::AssetBrowserEditorWindowFolderPane::ClearSelection()
{
    // 選択中のファイルパスリストをクリアする
    m_selectedFilePathList.clear();

    // 範囲選択の開始地点をクリアする
    m_rangeSelectionStartPath.clear();
}

nlohmann::json FWK::Editor::AssetBrowserEditorWindowFolderPane::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

void FWK::Editor::AssetBrowserEditorWindowFolderPane::AddFolderOpenState(const std::filesystem::path& a_folderPath, const bool a_isOpen)
{
    std::error_code l_errorCode = {};

    if (a_folderPath.empty() ||
        !std::filesystem::exists(a_folderPath, l_errorCode)) 
    {
        return; 
    }

    FWK_ASSERT_RETURN_IF(l_errorCode, "フォルダパスの存在確認に失敗しました。");

    m_folderOpenStateMap.try_emplace(a_folderPath, a_isOpen);
}

const std::filesystem::path& FWK::Editor::AssetBrowserEditorWindowFolderPane::FetchREFOperationTargetFolderPath() const
{
    // 選択中フォルダが倍場合はAssetルートを返す
    // Constant::k_assetRootFolderPathはinline const なので参照返し可能
    if (m_selectedFilePathList.empty()) { return Constant::k_assetRootFolderPath; }

    // 選択中フォルダの最後を返す
    // vector::back()は要求へのconst 参照を返すためコピー発生なし
    return m_selectedFilePathList.back();
}

void FWK::Editor::AssetBrowserEditorWindowFolderPane::DrawTreeNode(const std::filesystem::path& a_currentFolderPath, AssetBrowserEditorWindow& a_editorWindow)
{
    const auto& l_folderHierarchyMap = a_editorWindow.GetREFFolderHierarchyMap();
    const auto& l_folderHierarchyITR = l_folderHierarchyMap.find(a_currentFolderPath);

    // 子フォルダが存在するかどうか
    // マップにエントリが存在しない、またはエントリのvectorが空の場合は子なし
    // キーに対応したフォルダパスリストが存在し空の配列でなければ子フォルダが存在するため
    // リーフノード扱いしないかどうかの判定に使える
    const bool l_hasChild = (l_folderHierarchyITR != l_folderHierarchyMap.end()) &&
                             !l_folderHierarchyITR->second.empty();
   
    // TreeNode用Flagの組み立て
    // WorldOutlinerEditorWindow::DrawSceneNodeと同じパターン
    // SpanAvailWidth    : ノードのクリック範囲をウィンドウ幅いっぱいまで広げる
    // OpenOnArrow       : 矢印部分をクリックした場合のみ開閉する
    // OpenOnDoubleClick : ダブルクリックで開閉する(シングルクリックでは開閉しない)
    ImGuiTreeNodeFlags l_treeNodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth    |
                                         ImGuiTreeNodeFlags_OpenOnArrow       |
                                         ImGuiTreeNodeFlags_OpenOnDoubleClick;

    // もし子を持たなければリーフノードとして扱う
    if (!l_hasChild)
    {
        l_treeNodeFlags |= ImGuiTreeNodeFlags_Leaf |
                           ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }
    // 初回描画時にm_folderOpenStateMapの状態をImGuiへ反映
    // ImGuiCond_FirstUseEver : このアイテムで初回のみ設定
    // 2回目以降はImGuiの内部状態が使われる
    // m_folderOpenStateMapはTreeNodeExの戻り値で毎フレーム更新されるため
    // シリアライズ時に正しい状態が保存される
    else
    {
        ImGui::SetNextItemOpen(IsFolderOpen(a_currentFolderPath), ImGuiCond_None);
    }

    // 選択状態をフラグへ反映
    // m_selectedFilePathListにa_currentFolderPathが含まれていれば選択状態
    // std::findで線形探索(フォルダ選択は同時に数十件程度のため問題ない)
    const bool l_isSelected = std::find(m_selectedFilePathList.begin(), m_selectedFilePathList.end(), a_currentFolderPath) != m_selectedFilePathList.end();

    if (l_isSelected)
    {
        l_treeNodeFlags |= ImGuiTreeNodeFlags_Selected;
    }

          int   l_popStyleColorNUM = k_initialTreeNodePopStyleColorPaneActiveNUM;
    const auto& l_clipboard        = a_editorWindow.GetREFClipboard();
    const bool  l_isCutTarget      = (l_clipboard.GetVALOperationType() == Enum::AssetBrowserFileClipboardOperationType::Cut) && 
                                      l_clipboard.Contains(a_currentFolderPath);

    
    // ハイライト強弱の制御
    // a_activePane == FolderPane : 強ハイライト(ImGuiのデフォルト色)
    // a_activePane == AssetPane  : 弱ハイライト(デフォルト色のalphaを下げる)
    // a_activePane == Invalid    : 弱ハイライト(AssetPaneと同じ扱い)
    // ImGui::PushStyleColorは色をスタックへ積む
    // ImGuiCol_Header        : 選択時の背景色
    // ImGuiCol_HeaderHovered : ホバー時の背景色
    // ImGuiCol_HeaderActive  : クリック中の背景色
    // 3色分PushするのでPopStyleColor(3)で3つまとめて戻す
    if (a_editorWindow.GetVALActivePane() == Enum::AssetBrowserActivePaneType::FolderPane)
    {
        const auto&  l_headerColor       = l_isCutTarget ? Constant::k_imguiDarkBlueTranslucentColor : Constant::k_imguiStrongBlueColor;
        const auto&  l_headerActiveColor = l_isCutTarget ? Constant::k_imguiDarkBlueTranslucentColor : Constant::k_imguiStrongBlueColor;
              ImVec4 l_hoveredColor      = {};

        if (l_isCutTarget)
        {
            l_hoveredColor = Constant::k_imguiDarkBlueTranslucentColor;
        }
        else if (l_isSelected)
        {
            l_hoveredColor = Constant::k_imguiStrongBlueColor;
        }
        else
        {
            l_hoveredColor = Constant::k_imguiLightGrayColor;
        }

        ImGui::PushStyleColor(ImGuiCol_Header,        l_headerColor);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, l_hoveredColor);
        ImGui::PushStyleColor(ImGuiCol_HeaderActive,  l_headerActiveColor);

        l_popStyleColorNUM = k_treeNodePopStyleColorPaneActiveNUM;
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Header, Constant::k_imguiStrongBlueTranslucentColor);

        l_popStyleColorNUM = k_treeNodePopStyleColorPaneInactiveNUM;
    }

    // Cut中のフォルダはアイコン・フォルダ名も半透明にする
    /// ImGuiCol_Text : テキスト色(アイコン + フォルダ名前)
    // PushStyleColorでテキスト色を半透明にする
    if (l_isCutTarget)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, Constant::k_imguiCutTargetTextColor);

        // テキスト色のPush分をPopStyleColorの数に追加する
        ++l_popStyleColorNUM;
    }

    // リネームモードかどうか
    // m_renameState.m_isActiveがtrueかつ
    // 対象パスが現在描画中のノードと一致する場合
    const auto& l_constRenameState = a_editorWindow.GetREFRenameState();
          
    const bool l_isRenaming = l_constRenameState.m_isActive &&
                              l_constRenameState.m_targetFilePath == a_currentFolderPath;

    // フォルダアイコン + フォルダ名のラベルを構築
    const bool  l_isOpen = IsFolderOpen(a_currentFolderPath);
    const auto& l_icon   = (l_isOpen && 
                            l_hasChild) ? Constant::k_imguiFontAwesomeFolderOpenIcon : Constant::k_imguiFontAwesomeFolderCloseIcon;

    // フォルダ名はfilename()で取得(パスの最後の要素)
    // ルートフォルダの場合はfilename()が"Asset"になる
    const auto& l_label = l_isRenaming ? std::string{l_icon} + k_renameInputTextLabel.data() : std::string{l_icon} + " " + a_currentFolderPath.filename().string();

    // TreeNodeExでノードを描画
    // 戻り値 : ノードが開かれている場合はtrue、閉じている場合はfalse
    // Leaf + NoTreePushOnOpenの場合は常にfalseが返る
    const bool l_isNodeOpen = ImGui::TreeNodeEx(l_label.c_str(), l_treeNodeFlags);

    // リネームモード時は同じ行でInputTextを描画
    // TreeNodeExのラベル部分(アイコンの右)にInputTextを重ねる
    if (l_isRenaming)
    {
        auto& l_assetFilePathRegistry = a_editorWindow.GetMutableREFAssetFilePathRegistry();
        auto& l_fileOperation         = a_editorWindow.GetMutableREFFileOperation        ();
        auto& l_renameState           = a_editorWindow.GetMutableREFRenameState          ();

        // ImGui::SameLine : 同じ行に次のアイテムを配置
        // TreeNodeExのアイコンの右側にInputTextを配置する
        ImGui::SameLine();

        // ImGui::AlignTextToFramePadding : 
        // TreeNodeExはテキストベースで描画されるが
        // InputTextはフレーム付きで描画されるため高さが子おtなる
        // これを呼ぶことでInputTextの垂直位置を
        // TreeNodeExのテキストベースラインに合わせる
        ImGui::AlignTextToFramePadding();
    
        // 初回フレームのみフォーカスを当てる
        // m_isActiveがtrueになった直後の1フレーム目は
        // m_isFocusedがfalseのままなのでフォーカスを当てる
        // ImGui::SetKeyboardFocusHere(0) : 次に描画されるアイテムにフォーカスを当てる
        // InputTextの直前に呼ぶ必要がある
        if (!l_constRenameState.m_isFocused)
        {
            ImGui::SetKeyboardFocusHere(k_keyboardFocusNextItem);
        }

        const auto& l_style = ImGui::GetStyle();

        // InputTextのフレームパディングを小さくして
        // TreeNodeExのテキスト高さに近づける
        // デフォルトのFramePadding.yは3-4pxだが
        // TreeNodeExはテキストベースなので高さが低い
        // Y方向のパディングを1pxにして高さ合わせる
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(l_style.FramePadding.x, Constant::k_imguiInputTextHightPaddingAlignTreeNodeHight));

        // InputTextの幅を残り領域いっぱいに広げる
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

        // InputTextを描画
        // ImGuiInputTextFlags_EnterReturnsTrue : Enter押下でtrueを返す
        // ImGuiInputTextFlags_AutoSelectAll    : フォーカス時に全テキストを選択
        // バッファサイズはm_inputBuffer.size()で取得
        const bool l_isEnterPressed = ImGui::InputText(k_renameInputTextLabel.data(),
                                                       l_renameState.m_inputBuffer.data(),
                                                       l_renameState.m_inputBuffer.size(),
                                                       ImGuiInputTextFlags_EnterReturnsTrue |
                                                       ImGuiInputTextFlags_AutoSelectAll);

        ImGui::PopStyleVar();

        // フォーカスされたらフラグを立てる
        if (ImGui::IsItemFocused())
        {
            l_renameState.m_isFocused = true;
        }

        // 空白クリック検知
        // ImGui::IsWindowHovered   : このChildWindow上にマウスがあるか
        // !ImGui::IsAnyItemHovered : いずれこのアイテム上にもマウスがない = 空白
        // ImGui::IsMouseClicked    : このフレームで左クリックされた
        // これにより空白をクリックしたときにリネームを確定する
        const bool l_isEmptySpaceClick = ImGui::IsWindowHovered()   &&
                                         !ImGui::IsAnyItemHovered() &&
                                         ImGui::IsMouseClicked(ImGuiMouseButton_Left);

        // リネーム確定条件
        // 1.Enter押下
        // 2.フォーカル消失(別の場所をクリック等)
        // m_isFocusedがtrue(=過去にフォーカスされた)状態で
        // 現在フォーカスされていない場合を確定とみなす
        if (l_isEnterPressed          ||
           (l_renameState.m_isFocused &&
           !ImGui::IsItemFocused())   ||
            l_isEmptySpaceClick)
        {
            // InputTextの内容を取得
            // data()で先頭ポインタを取得し、std::stringを構築
            // から文字列の場合はリネームしない
            if (const auto& l_newName = std::string(l_renameState.m_inputBuffer.data());
                !l_newName.empty())
            {
                // FileOperation::Renameでファイルシステム上でリネーム
                // 同名衝突時は自動で番号付与される
                l_fileOperation.Rename(a_currentFolderPath, l_newName, l_assetFilePathRegistry);
            }

            // リネームモードを終了
            l_renameState.m_isActive  = false;
            l_renameState.m_isFocused = false;
        }
    }

    // 開閉状態をm_folderOpenStateMapへ反映
    // 毎フレームTreeNodeExの戻り値で更新する
    // これによりImGuiとm_folderOpenStateMapが同期する
    // リーフノードの場合はTreeNodeExが常にfalseを返すため更新しない
    if (l_hasChild)
    {
        m_folderOpenStateMap[a_currentFolderPath] = l_isNodeOpen;
    }

    // 左クリック : 選択
    // 仕様 : 左クリック一回で現在選択中のファイルパスとして扱う
    //        開閉はImGuiが管理するためToggleFolderOpenは呼ばない
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
        // ImGui::GetIO().KeyShift : Shiftキーが押されているか
        // ImGui::GetIO().KeyCtrl  : Ctrlキーが押されているか
        const auto& l_io = ImGui::GetIO();

        SelectFolder(l_folderHierarchyMap, 
                     a_currentFolderPath,
                     l_io.KeyShift,
                     l_io.KeyCtrl);
    }

    const auto& l_contextMenuOpenPopupLabel = std::string{ k_contextMenuOpenPopupLabel } + a_currentFolderPath.string();
    const auto& l_popupDrawer               = a_editorWindow.GetREFPopupDrawer                                       ();

    // 右クリック : 選択 + ポップアップ
    // 仕様 : フォルダ上を右クリックで現在選択中のフォルダとして扱い
    //        右ポップアップを開く
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
    {
        // 右クリックしたフォルダが既に選択リストに含まれているか確認
        // 未選択の場合のみSelectFolderを読んで選択状態を更新する
        // 既に選択されている場合は選択状態を維持する
        if (const bool l_isAlreadySelected = std::find(m_selectedFilePathList.begin(), m_selectedFilePathList.end(), a_currentFolderPath) != m_selectedFilePathList.end();
            !l_isAlreadySelected)
        {
            SelectFolder(l_folderHierarchyMap, a_currentFolderPath);
        }

        // BeginPopupで右クリック用ポップアップを開く
        l_popupDrawer.BeginPopup(l_contextMenuOpenPopupLabel);
    }

    // ポップアップ描画
    // BeginPopupで指定したラベルのポップアップを開く
    // ポップアップIDはOpenPopupで指定したものと同じものを使う必要がある
    l_popupDrawer.Draw(m_selectedFilePathList,
                       a_currentFolderPath,
                       l_contextMenuOpenPopupLabel,
                       Enum::AssetBrowserPopupContextType::FolderPane_OnFolder,
                       a_editorWindow);

    // Pushした数文をまとめてPopする
    ImGui::PopStyleColor(l_popStyleColorNUM);

    // 子ディレクトリを再帰描画
    if (!l_isNodeOpen ||
        !l_hasChild)
    {
        return;
    }

    // l_hierarchyITRが有効な間に->secondにアクセスする
    // l_hasChildがtrueの場合のみここに到達するため
    // l_hierarchyITRは必ずend()ではない
    for (const auto& l_childFolder : l_folderHierarchyITR->second)
    {
        DrawTreeNode(l_childFolder, a_editorWindow);
    }

    // TreeNodeExによってインデントが一段下がっているため
    // TreePopで一段戻す
    ImGui::TreePop();    
}

void FWK::Editor::AssetBrowserEditorWindowFolderPane::BuildDisplayedFolderList(const std::unordered_map<std::filesystem::path, std::vector<std::filesystem::path>>& a_folderHierarchyMap, const std::filesystem::path& a_folderPath, std::vector<std::filesystem::path>& a_displayedList)
{
    // 自分自身をリストへ追加
    // 呼び出し側で渡されたvectorへ表示順に追加していく
    a_displayedList.emplace_back(a_folderPath);

    // 開いているフォルダの子は表示されないためスキップ
    // m_folderOpenStateMapはFolderPaneのメンバなので参照可能
    if (!IsFolderOpen(a_folderPath)) { return; }

    // 子フォルダリストを追加
    const auto& l_itr = a_folderHierarchyMap.find(a_folderPath);

    // 子がない場合は終了
    if (l_itr == a_folderHierarchyMap.end()) { return; }

    // 子フォルダに対して再帰的に呼びだし
    for (const auto& l_chldPath : l_itr->second)
    {
        BuildDisplayedFolderList(a_folderHierarchyMap, l_chldPath, a_displayedList);
    }
}

void FWK::Editor::AssetBrowserEditorWindowFolderPane::SelectFolder(const std::unordered_map<std::filesystem::path, std::vector<std::filesystem::path>>& a_folderHierarchyMap, 
                                                                   const std::filesystem::path&                                                         a_folderPath, 
                                                                   const bool                                                                           a_isRangeSelection, 
                                                                   const bool                                                                           a_isToggleSelection)
{
    // Shift + Click : 範囲選択
    // m_rangeSelectionStartPathからa_folderPathまでの範囲を選択する
    // ツリー上の表示順で範囲を選択する
    if (a_isRangeSelection)
    {
        // 範囲選択の開始地点が未設定の場合は単一選択として扱う
        if (m_rangeSelectionStartPath.empty())
        {
            // 通常クリック : 選択をクリアして単一選択
            m_selectedFilePathList.clear       ();
            m_selectedFilePathList.emplace_back(a_folderPath);

            // 現在フォルダを更新
            // 単一選択時は現在フォルダを選択フォルダにする
            // AssetPaneはこのm_currentFolderPathを参照して内容を表示する
            m_currentFolderPath = a_folderPath;

            // 範囲選択の開始地点を更新
            m_rangeSelectionStartPath = a_folderPath;
        }
        else
        {
            // 表示中のフォルダリストを構築
            // BuildDisplayedFolderListは現在開いているフォルダツリーを
            // 表示順(上から下)に並べたリストを返す
            // これを使って開始地点からクリック店までの範囲を決定する
            std::vector<std::filesystem::path> l_displayedFolderList = {};

            BuildDisplayedFolderList(a_folderHierarchyMap, Constant::k_assetRootFolderPath, l_displayedFolderList);

            // 開始地点とクリック店のリスト内インデックスを検索
            auto l_startITR = std::find(l_displayedFolderList.begin(), l_displayedFolderList.end(), m_rangeSelectionStartPath);
            auto l_endITR   = std::find(l_displayedFolderList.begin(), l_displayedFolderList.end(), a_folderPath); 

            // 両方がリストに存在する場合のみ範囲選択を行う
            if (l_startITR != l_displayedFolderList.end() &&
                l_endITR != l_displayedFolderList.end())
            {
                // 開始地点がクリック点より後ろの場合は入れ替える
                // 例 : Scene(開始) -> Data(クリック)の場合は
                // Data -> Sceneの順に選択する
                if (l_startITR > l_endITR)
                {
                    std::swap(l_startITR, l_endITR);
                }

                // 開始地点からクリック店まで(両端含む)を選択リストへ追加
                m_selectedFilePathList.clear();

                for (auto l_itr = l_startITR; l_itr <= l_endITR; ++l_itr)
                {
                    m_selectedFilePathList.emplace_back(*l_itr);
                }
            }
            else
            {
                // リストに存在しない場合は簡易的に二つだけ選択(フォールバック)
                m_selectedFilePathList.clear();
                m_selectedFilePathList.emplace_back(m_rangeSelectionStartPath);
                m_selectedFilePathList.emplace_back(a_folderPath);
            }
        }

        return;
    }

    // Ctrl + Click : 選択/選択解除のトグル
    // 既に選択されている場合は選択解除
    // 選択されていない場合は選択に追加
    if (a_isToggleSelection)
    {
        // std::findで選択リスト内にa_folderPathが存在するか検索
        // 選択リストはvectorなので線形探索になるが
        // フォルダ選択は同時に数十件程度のため実用上問題ない
        if (auto l_itr = std::find(m_selectedFilePathList.begin(), m_selectedFilePathList.end(), a_folderPath);
            l_itr != m_selectedFilePathList.end())
        {
            // 既に選択されている場合は選択解除
            m_selectedFilePathList.erase(l_itr);

            // 選択解除後、残りが一つの場合は現在フォルダを更新
            // 残りが0件または複数件の場合は更新しない
            // (複数選択時は操作無効、0件時は前の現在フォルダを維持)
            if (m_selectedFilePathList.size() == Constant::k_editorSelectedFolderSingleSize)
            {
                m_currentFolderPath = m_selectedFilePathList.back();
            }
        }
        else
        {
            // 選択されていない場合は選択に追加
            m_selectedFilePathList.emplace_back(a_folderPath);

            // 追加後、選択数が1件の場合は現在フォルダを更新
            if (m_selectedFilePathList.size() == Constant::k_editorSelectedFolderSingleSize)
            {
                m_currentFolderPath = a_folderPath;
            }
        }

        return;
    }

    // 通常クリック : 選択をクリアして単一選択
    m_selectedFilePathList.clear       ();
    m_selectedFilePathList.emplace_back(a_folderPath);

    // 現在フォルダを更新
    // 単一選択時はこ現在フォルダを選択フォルダにする
    // AssetPaneはこのm_currentFolderPathを参照して内容を表示する
    m_currentFolderPath = a_folderPath;

    // 範囲選択の開始地点を更新
    m_rangeSelectionStartPath = a_folderPath;
}

bool FWK::Editor::AssetBrowserEditorWindowFolderPane::IsFolderOpen(const std::filesystem::path& a_folderPath) const
{
    // m_folderOpenStateMapからa_folderPathの開閉状態を取得する
    // std::unordered_map::findはキーが存在しない場合end()を返す
    // 存在しない場合はデフォルトで「閉じている(false)」として扱う
    const auto& l_itr = m_folderOpenStateMap.find(a_folderPath);

    // エントリが存在しない場合はfalse(閉じている)を返す
    if (l_itr == m_folderOpenStateMap.end()) { return false; }

    return l_itr->second;
}

void FWK::Editor::AssetBrowserEditorWindowFolderPane::ToggleFolderOpen(const std::filesystem::path& a_folderPath)
{
    // 現在の開閉状態を反転させる
    // m_folderOpenStateMapにエントリが存在しない場合は
    // デフォルトで閉じているとみなし、開く(true)にする
    const auto& l_itr = m_folderOpenStateMap.find(a_folderPath);

    if (l_itr == m_folderOpenStateMap.end()) 
    {
        // エントリが存在しない場合は新規作成してtrue(開く)をセット
        m_folderOpenStateMap.try_emplace(a_folderPath, true);

        return;
    }

    // エントリが存在する場合は反転させる
    l_itr->second = !l_itr->second;
}