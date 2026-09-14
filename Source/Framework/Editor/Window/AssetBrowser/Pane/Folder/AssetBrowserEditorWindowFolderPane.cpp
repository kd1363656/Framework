#include "AssetBrowserEditorWindowFolderPane.h"

void FWK::Editor::AssetBrowserEditorWindowFolderPane::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::Editor::AssetBrowserEditorWindowFolderPane::Draw(const std::unordered_map<std::filesystem::path, std::vector<std::filesystem::path>>& a_folderHierarchyMap,
                                                           const AssetBrowserEditorWindowPopupDrawer&                                           a_popupDrawer,
                                                           const AssetBrowserEditorWindowAssetCreator&                                          a_assetCreator,
                                                           const std::filesystem::path&                                                         a_assetRootFolderPath, 
                                                           const float                                                                          a_paneWidth, 
                                                                 Enum::AssetBrowserActivePaneType&                                              a_activePane,
                                                                 AssetBrowserEditorWindowFileOperation&                                         a_fileOperation, 
                                                                 AssetBrowserEditorWindowClipboard&                                             a_clipboard, 
                                                                 AssetFilePathRegistry&                                                         a_assetFilePathRegistry,
                                                                 Struct::AssetBrowserEditorWindowRenameState&                                   a_renameState)
{
    // ImGui::BeginChild()は
    // 現在のWindowの内部にもう一つの描画領域を作成するAPI
    if (const ImVec2 l_folderPaneSize = { a_paneWidth, Constant::k_imguiRemainingSize.y };
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
        a_activePane = Enum::AssetBrowserActivePaneType::FolderPane;
    }

    ImGui::TextUnformatted(k_paneTitleLabel.data());
    ImGui::Separator      ();

    // ルートフォルダから再帰的にツリーノードを描画
    // a_assetRootFolderPathがツリーのルート(Assetフォルダ)
    // DrawTreeNodeはa_folderPath自身を1ノード描画し
    // 開いている場合は子ディレクトリに対して再帰的にDrawTreeNodeを呼ぶ
    DrawTreeNode(a_folderHierarchyMap,
                 a_popupDrawer,
                 a_assetCreator,
                 a_assetRootFolderPath,
                 a_assetRootFolderPath,
                 a_activePane,
                 a_paneWidth,
                 a_fileOperation,
                 a_clipboard,
                 a_assetFilePathRegistry,
                 a_renameState);

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
        a_popupDrawer.BeginPopup(k_emptySpaceContextMenuOpenLabel);
    }

    // 対象フォルダ : 選択中フォルダ、なければAssetルート
    const auto& l_targetFolder = m_selectedFilePathList.empty() ? a_assetRootFolderPath : m_selectedFilePathList.back();

    // 空スペース用ポップアップ描画
    // PopupDrawer : Draw内部でBeginPopup / EndPopupを呼ぶ
    // ポップアップが開いていない場合はDraw内部でreturnする
    a_popupDrawer.Draw(m_selectedFilePathList,
                       a_assetCreator,
                       l_targetFolder,
                       l_targetFolder,
                       k_emptySpaceContextMenuOpenLabel,
                       Enum::AssetBrowserPopupContextType::FolderPane_OnFolder,
                       a_fileOperation,
                       a_clipboard,
                       a_assetFilePathRegistry,
                       a_renameState);

    ImGui::EndChild();
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

void FWK::Editor::AssetBrowserEditorWindowFolderPane::DrawTreeNode(const std::unordered_map<std::filesystem::path, std::vector<std::filesystem::path>>& a_folderHierarchyMap,
                                                                   const AssetBrowserEditorWindowPopupDrawer&                                           a_popupDrawer,
                                                                   const AssetBrowserEditorWindowAssetCreator&                                          a_assetCreator, 
                                                                   const std::filesystem::path&                                                         a_currentFolderPath,
                                                                   const std::filesystem::path&                                                         a_assetRootFolderPath,
                                                                   const Enum::AssetBrowserActivePaneType                                               a_activePane,
                                                                   const float                                                                          a_paneWidth, 
                                                                         AssetBrowserEditorWindowFileOperation&                                         a_fileOperation, 
                                                                         AssetBrowserEditorWindowClipboard&                                             a_clipboard, 
                                                                         AssetFilePathRegistry&                                                         a_assetFilePathRegistry,
                                                                         Struct::AssetBrowserEditorWindowRenameState&                                   a_renameState)
{
    const auto& l_folderHierarchyITR = a_folderHierarchyMap.find(a_currentFolderPath);

    // 子フォルダが存在するかどうか
    // マップにエントリが存在しない、またはエントリのvectorが空の場合は子なし
    // キーに対応したフォルダパスリストが存在し空の配列でなければ子フォルダが存在するため
    // リーフノード扱いしないかどうかの判定に使える
    const bool l_hasChild = (l_folderHierarchyITR != a_folderHierarchyMap.end()) &&
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
    if (std::find(m_selectedFilePathList.begin(), m_selectedFilePathList.end(), a_currentFolderPath) != m_selectedFilePathList.end())
    {
        l_treeNodeFlags |= ImGuiTreeNodeFlags_Selected;
    }

    // ハイライト強弱の制御
    // a_activePane == FolderPane : 強ハイライト(ImGuiのデフォルト色)
    // a_activePane == AssetPane  : 弱ハイライト(デフォルト色のalphaを下げる)
    // a_activePane == Invalid    : 弱ハイライト(AssetPaneと同じ扱い)
    // ImGui::PushStyleColorは色をスタックへ積む
    // ImGuiCol_Header        : 選択時の背景色
    // ImGuiCol_HeaderHovered : ホバー時の背景色
    // ImGuiCol_HeaderActive  : クリック中の背景色
    // 3色分PushするのでPopStyleColor(3)で3つまとめて戻す
    const bool l_isStrongHighlight = (a_activePane == Enum::AssetBrowserActivePaneType::FolderPane);

    if (!l_isStrongHighlight)
    {
        // 弱ハイライト : デフォルト色のalphaを0.5倍にする
        auto l_headerColor        = ImGui::GetStyleColorVec4(ImGuiCol_Header);
        auto l_headerHoveredColor = ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered);
        auto l_headerActiveColor  = ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive);

        l_headerColor.w        *= Constant::k_imguiHighLightHalfStrength;
        l_headerHoveredColor.w *= Constant::k_imguiHighLightHalfStrength;
        l_headerActiveColor.w  *= Constant::k_imguiHighLightHalfStrength;

        // alphaを0.5倍にして薄くする
        ImGui::PushStyleColor(ImGuiCol_Header,        l_headerColor);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, l_headerHoveredColor);
        ImGui::PushStyleColor(ImGuiCol_HeaderActive,  l_headerActiveColor);
    }

    // フォルダアイコン + フォルダ名のラベルを構築
    const bool  l_isOpen = IsFolderOpen(a_currentFolderPath);
    const auto& l_icon   = (l_isOpen && 
                            l_hasChild) ? Constant::k_imguiFontAwesomeFolderOpenIcon : Constant::k_imguiFontAwesomeFolderCloseIcon;

    // フォルダ名はfilename()で取得(パスの最後の要素)
    // ルートフォルダの場合はfilename()が"Asset"になる
    const auto& l_label = std::string{ l_icon } + " " + a_currentFolderPath.filename().string();

    // TreeNodeExでノードを描画
    // 戻り値 : ノードが開かれている場合はtrue、閉じている場合はfalse
    // Leaf + NoTreePushOnOpenの場合は常にfalseが返る
    const bool l_isNodeOpen = ImGui::TreeNodeEx(l_label.c_str(), l_treeNodeFlags);

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

        SelectFolder(a_currentFolderPath, l_io.KeyShift, l_io.KeyCtrl);
    }

    const auto& l_contextMenuOpenPopupLabel = std::string{ k_contextMenuOpenPopupLabel } + a_currentFolderPath.string();

    // 右クリック : 選択 + ポップアップ
    // 仕様 : フォルダ上を右クリックで現在選択中のフォルダとして扱い
    //        右ポップアップを開く
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
    {
        SelectFolder(a_currentFolderPath);

        // BeginPopupで右クリック用ポップアップを開く
        a_popupDrawer.BeginPopup(l_contextMenuOpenPopupLabel);
    }

    // ポップアップ描画
    // BeginPopupで指定したラベルのポップアップを開く
    // ポップアップIDはOpenPopupで指定したものと同じものを使う必要がある
    a_popupDrawer.Draw(m_selectedFilePathList,
                       a_assetCreator,
                       a_currentFolderPath,
                       a_currentFolderPath,
                       l_contextMenuOpenPopupLabel,
                       Enum::AssetBrowserPopupContextType::FolderPane_OnFolder,
                       a_fileOperation,
                       a_clipboard,
                       a_assetFilePathRegistry,
                       a_renameState);

    // ハイライト色をスタックから戻す
    if (!l_isStrongHighlight)
    {
        // Pushした3色分をまとめてPopする
        ImGui::PopStyleColor(k_treeNodePopStyleColorNUM);
    }

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
        DrawTreeNode(a_folderHierarchyMap,
                     a_popupDrawer,
                     a_assetCreator,
                     l_childFolder,
                     a_assetRootFolderPath,
                     a_activePane,
                     a_paneWidth,
                     a_fileOperation,
                     a_clipboard,
                     a_assetFilePathRegistry,
                     a_renameState);
    }

    // TreeNodeExによってインデントが一段下がっているため
    // TreePopで一段戻す
    ImGui::TreePop();
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

void FWK::Editor::AssetBrowserEditorWindowFolderPane::SelectFolder(const std::filesystem::path& a_folderPath, const bool a_isRangeSelection, const bool a_isToggleSelection)
{
    // Shift + Click : 範囲選択
    // m_rangeSelectionStartPathからa_folderPathまでの範囲を選択する
    // ツリー上の順番で範囲を選択する必要があるが
    // ツリー順序の取得は描画時しかできないため
    // ここでは簡易的にa_folderPathを選択リストへ追加する
    // 本格的な範囲選択はDrawTreeNode側で処理する
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
            // 開始地点とクリック点を両方選択
            m_selectedFilePathList.clear       ();
            m_selectedFilePathList.emplace_back(m_rangeSelectionStartPath);
            m_selectedFilePathList.emplace_back(a_folderPath);
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