#include "AssetBrowserEditorWindowAssetPane.h"

void FWK::Editor::AssetBrowserEditorWindowAssetPane::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::Editor::AssetBrowserEditorWindowAssetPane::Draw(AssetBrowserEditorWindow& a_editorWindow)
{
    if (!ImGui::BeginChild(k_childLabel.data(), Constant::k_imguiRemainingSize, true))
    {
        ImGui::EndChild();

        return;
    }

    // アセットペイン上でクリックされた場合、アクティブPaneをAssetPaneにする
    // これによりショートカットキーがアセットペイン向けの処理を行う
    // ImGui::IsWindowHovered : このChildWindow上にマウスがあるか
    // ImGui::IsMouseClicked  : このフレームでクリックされたか
    // 左クリック・右クリックどちらもアクティブPaneを切り替える
    if (ImGui::IsWindowHovered() &&
        (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || 
         ImGui::IsMouseClicked(ImGuiMouseButton_Right)))
    {
        a_editorWindow.SetActivePane(Enum::AssetBrowserActivePaneType::AssetPane);
    }

    ImGui::TextUnformatted(k_paneTitleLabel.data());
    ImGui::Separator      ();

    // パンくずリストを描画、所定フォルダパスをクリックで
    // 現在参照中のフォルダを切り替えれる
    m_breadcrumb.Draw(a_editorWindow);
   
    // 表示中ファイルパスリストを構築
    // FolderPanneの選択状態に応じて点一フォルダ of 複数フォルダマージ
    std::vector<std::filesystem::path> l_displayedFilePathList = {};

    BuildDisplayedFilePathList(a_editorWindow, l_displayedFilePathList);

    // 一行あたりのカード数を計算
    // カード幅 + カード間余白で一行に何枚ならぶか算出
    const float l_availableWidth = ImGui::GetContentRegionAvail().x;
    const int   l_cardsPerRow    = CalculateCardPerRow         (l_availableWidth);

    // カードをグリッド描画
    // 左 -> 右に並べ、1行の枚数に達したら改行
    const auto& l_popupDrawer = a_editorWindow.GetREFPopupDrawer();
          int   l_column      = k_initialCardColumn;

    for (const auto& l_filePath : l_displayedFilePathList)
    {
        // 2枚目以降は同じ行配置
        if (l_column > k_initialCardColumn)
        {
            ImGui::SameLine(Constant::k_imguiRemainingSize.x, k_cardSpacing);
        }

        // 表示中リストをDrawCardへ渡す
        // クリック時の範囲選択でリスト内インデックス検索に使う
        DrawCard(l_displayedFilePathList, l_filePath, a_editorWindow);

        ++l_column;

        // 1行の枚数に達したら改行(SameLineを呼ばないことで次行へ)
        if (l_column >= l_cardsPerRow)
        {
            l_column = k_initialCardColumn;
        }
    }

    // 空白スペース左クリック = 選択解除
    // ImGui::IsWindowHovered   : このChildWindow上にマウスがあるか
    // !ImGui::IsAnyItemHovered : いずれのアイテム(カード)上にマウスがない = 空白
    // ImGui::IsMouseClicked    : このフレームで左クリックされたか
    if (ImGui::IsWindowHovered()   &&
        !ImGui::IsAnyItemHovered() &&
        ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        m_selectionState.ClearSelection();
        m_currentCursorFilePath.clear  ();
    }

    // 空白スペース右クリック = コンテキストメニュー
    // 式フォルダ・新規プレハブ・新規シーン作成が可能
    if (ImGui::IsWindowHovered()   &&
        !ImGui::IsAnyItemHovered() &&
        ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    {
        l_popupDrawer.BeginPopup(k_emptySpaceContextMenuLabel);
    }

    // 空白スペース用ポップアップ描画
    // 対象フォルダ : 現在参照中フォルダ、なければAssetルート
    const auto& l_selectedFilePathList    = m_selectionState.GetREFSelectedFilePathList ();
    const auto& l_currentSelectFolderPath = a_editorWindow.GetREFCurrentSelectFolderPath();
    const auto& l_targetFolder            = l_currentSelectFolderPath.empty() ? Constant::k_assetRootFolderPath : l_currentSelectFolderPath;

    l_popupDrawer.Draw(l_selectedFilePathList,
                       l_targetFolder,
                       k_emptySpaceContextMenuLabel,
                       Enum::AssetBrowserPopupContextType::AssetPane_OnEmpty,
                       a_editorWindow);

    ImGui::EndChild();
}

nlohmann::json FWK::Editor::AssetBrowserEditorWindowAssetPane::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

void FWK::Editor::AssetBrowserEditorWindowAssetPane::MoveSelectionUp(AssetBrowserEditorWindow& a_editorWindow, const bool a_isRangeSelection)
{
    std::vector<std::filesystem::path> l_displayedList = {};

    // 表示中ファイルパスリストを構築
    BuildDisplayedFilePathList(a_editorWindow, l_displayedList);

    if (l_displayedList.empty()) { return; }

    // カーソル位置を決定
    const auto& l_cursorPath = m_currentCursorFilePath.empty() ? l_displayedList.front() : m_currentCursorFilePath;

    // カーソル位置をリストから検索
    auto l_cursorITR = std::find(l_displayedList.begin(), l_displayedList.end(), l_cursorPath);

    if (l_cursorITR == l_displayedList.end())
    {
        // リストに見つからない場合は先頭を選択
        m_currentCursorFilePath = l_displayedList.front();

        SelectFile(l_displayedList, 
                   l_displayedList.front(),                    
                   a_isRangeSelection,
                   false);

        return;
    }

    // 1行分(CardsPerRow)前に移動
    const float l_availableWidth = ImGui::GetContentRegionAvail().x;
    const auto  l_cardsPerRow    = CalculateCardPerRow         (l_availableWidth);

    // インデックスを計算
    const auto l_cursorIndex = static_cast<std::uint32_t>(std::distance(l_displayedList.begin(), l_cursorITR));
    const auto l_newIndex    = l_cursorIndex - l_cardsPerRow;

    // 先頭より前なのか、サイズに収まっているかを確認する
    if (l_newIndex >= l_displayedList.size())
    {
        return; 
    }
    
    const auto& l_newPath = l_displayedList[l_newIndex];

    // 範囲選択モード
    if (a_isRangeSelection)
    {
        // アンカー未設定の場合は現在のカーソルをアンカーにする
        if (m_selectionState.GetREFRangeSelectionStartPath().empty())
        {
            m_selectionState.SetRangeSelectionStartPath(l_cursorPath);
        }
    }

    // カーソル更新
    m_currentCursorFilePath = l_newPath;

    // 選択実行
    SelectFile(l_displayedList,
               l_newPath,
               a_isRangeSelection,
               false);
}
void FWK::Editor::AssetBrowserEditorWindowAssetPane::MoveSelectionDown(AssetBrowserEditorWindow& a_editorWindow, const bool a_isRangeSelection)
{
    std::vector<std::filesystem::path> l_displayedList = {};

    BuildDisplayedFilePathList(a_editorWindow, l_displayedList);

    if (l_displayedList.empty()) { return; }

    const auto& l_cursorPath = m_currentCursorFilePath.empty() ? l_displayedList.front() : m_currentCursorFilePath;

    auto l_cursorITR = std::find(l_displayedList.begin(), l_displayedList.end(), l_cursorPath);

    if (l_cursorITR == l_displayedList.end())
    {
        m_currentCursorFilePath = l_displayedList.front();

        SelectFile(l_displayedList,
                   l_displayedList.front(),
                   a_isRangeSelection,
                   false);

        return;
    }

    const float l_availableWidth = ImGui::GetContentRegionAvail            ().x;
    const auto  l_cardsPerRow    = CalculateCardPerRow                     (l_availableWidth);
    const auto  l_cursorIndex    = static_cast<std::uint32_t>(std::distance(l_displayedList.begin(), l_cursorITR));
    const auto  l_newIndex       = l_cursorIndex + l_cardsPerRow;

    // 末尾を超えたら何もしない
    if (l_newIndex >= l_displayedList.size()) { return; }

    const auto& l_newPath = l_displayedList[l_newIndex];

    if (a_isRangeSelection)
    {
        if (const auto& l_rangeSelectionStartPath = m_selectionState.GetREFRangeSelectionStartPath();
            l_rangeSelectionStartPath.empty())
        {
            m_selectionState.SetRangeSelectionStartPath(l_cursorPath);
        }
    }

    m_currentCursorFilePath = l_newPath;

    SelectFile(l_displayedList,
               l_newPath,
               a_isRangeSelection,
               false);
}
void FWK::Editor::AssetBrowserEditorWindowAssetPane::MoveSelectionLeft(AssetBrowserEditorWindow& a_editorWindow, const bool a_isRangeSelection)
{
    std::vector<std::filesystem::path> l_displayedList = {};

    BuildDisplayedFilePathList(a_editorWindow, l_displayedList);

    if (l_displayedList.empty()) { return; }

    const auto& l_cursorPath = m_currentCursorFilePath.empty() ? l_displayedList.front() : m_currentCursorFilePath;
          auto  l_cursorITR  = std::find(l_displayedList.begin(), l_displayedList.end(), l_cursorPath);

    if (l_cursorITR == l_displayedList.end())
    {
        m_currentCursorFilePath = l_displayedList.front();

        SelectFile(l_displayedList, 
                   l_displayedList.front(),
                   a_isRangeSelection,
                   false);
    }

    // 既に先頭なら何もしない
    if (l_cursorITR == l_displayedList.begin()) { return; }

          auto  l_prevITR  = std::prev(l_cursorITR);
    const auto& l_prevPath = *l_prevITR;

    if (a_isRangeSelection)
    {
        if (const auto& l_rangeSelectionStartPath = m_selectionState.GetREFRangeSelectionStartPath();
            l_rangeSelectionStartPath.empty())
        {
            m_selectionState.SetRangeSelectionStartPath(l_cursorPath);
        }
    }

    m_currentCursorFilePath = l_prevPath;

    SelectFile(l_displayedList, 
               l_prevPath, 
               a_isRangeSelection,
               false);
}
void FWK::Editor::AssetBrowserEditorWindowAssetPane::MoveSelectionRight(AssetBrowserEditorWindow& a_editorWindow, const bool a_isRangeSelection)
{
    std::vector<std::filesystem::path> l_displayedList = {};

    BuildDisplayedFilePathList(a_editorWindow, l_displayedList);

    if (l_displayedList.empty()) { return; }

    const auto& l_cursorPath = m_currentCursorFilePath.empty() ? l_displayedList.front() : m_currentCursorFilePath;
          auto  l_cursorITR  = std::find(l_displayedList.begin(), l_displayedList.end(), l_cursorPath);

    auto l_nextITR = std::next(l_cursorITR);

    // 既に末尾なら何もしない
    if (l_nextITR == l_displayedList.end()) { return; }

    const auto& l_nextPath = *l_nextITR;

    if (a_isRangeSelection)
    {
        if (const auto& l_rangeSelectionStartPath = m_selectionState.GetREFRangeSelectionStartPath();
            l_rangeSelectionStartPath.empty())
        {
            m_selectionState.SetRangeSelectionStartPath(l_cursorPath);
        }
    }

    m_currentCursorFilePath = l_nextPath;

    SelectFile(l_displayedList,
               l_nextPath,
               a_isRangeSelection,
               false);
}

void FWK::Editor::AssetBrowserEditorWindowAssetPane::NavigateToCurrentCursor(AssetBrowserEditorWindow& a_editorWindow)
{
    if (m_currentCursorFilePath.empty()) { return; }

    std::error_code l_errorCode = {};

    // カーソル位置がフォルダの場合のみナビゲート
    if (std::filesystem::is_directory(m_currentCursorFilePath, l_errorCode))
    {
        NavigateToFolder(m_currentCursorFilePath, a_editorWindow);
    }
}

std::vector<std::filesystem::path> FWK::Editor::AssetBrowserEditorWindowAssetPane::FetchVALDisplayedFilePathList(AssetBrowserEditorWindow& a_editorWindow)
{
    std::vector<std::filesystem::path> l_displayedList = {};

    BuildDisplayedFilePathList(a_editorWindow, l_displayedList);

    return l_displayedList;
}

std::filesystem::path FWK::Editor::AssetBrowserEditorWindowAssetPane::FetchVALOperationTargetFilePath() const
{
    const auto& l_selectedFilePathList = m_selectionState.GetREFSelectedFilePathList();

    if (l_selectedFilePathList.empty()) { return {}; }

    // 選択中の最後を返す
    return l_selectedFilePathList.back();
}

void FWK::Editor::AssetBrowserEditorWindowAssetPane::DrawCard(const std::vector<std::filesystem::path>& a_displayedFilePathList, const std::filesystem::path& a_filePath, AssetBrowserEditorWindow& a_editorWindow)
{
    const auto& l_popupDrawer          = a_editorWindow.GetREFPopupDrawer();
    const auto& l_clipboard            = a_editorWindow.GetREFClipboard  ();
    const auto& l_renameState          = a_editorWindow.GetREFRenameState();
    const auto& l_selectedFilePathList = m_selectionState.GetREFSelectedFilePathList();

    // 選択状態判定
    // m_selectedFilePathListにa_filePathが含まれていれば選択状態
    const bool l_isSelected = std::find(l_selectedFilePathList.begin(), l_selectedFilePathList.end(), a_filePath) != l_selectedFilePathList.end();

    // 切り取り対象判定
    // クリップボードがCut操作で、かつこのファイルパスが含まれている場合
    const bool l_isCutTarget = (l_clipboard.GetVALOperationType() == Enum::AssetBrowserFileClipboardOperationType::Cut) &&
                               l_clipboard.Contains(a_filePath);

    // リネームモード判定
    // m_renameState.m_isActiveがtrueかつ対象パスがこのカードと一致するか
    const bool l_isRenaming = l_renameState.m_isActive &&
                              l_renameState.m_targetFilePath == a_filePath;

    const ImVec2& l_cardSize = { k_cardWidth, k_cardHeight };

    // カードを上座標(Screen座標)
    // InvisibleButton描画時に取得することで
    // DrawListによる描画座標として扱う
    const auto&   l_cardMIN = ImGui::GetCursorScreenPos();
    const ImVec2& l_cardMAX = { l_cardMIN.x + l_cardSize.x, l_cardMIN.y + l_cardSize.y };

    // InvisibleButtonでインタラクティブな霊異記を確保
    // 実際の描画はDrawListで行うため、ボタンは透明
    // ラベルは一意になるようにファイルパスを付加
    const auto l_label = std::string{ k_cardPrefixLabel } + a_filePath.generic_string();

    ImGui::InvisibleButton(l_label.c_str(), l_cardSize);

    // ホバー判定
    const bool l_isHovered = ImGui::IsItemHovered();

    // アクティブペイン判定
    // AssetPaneがアクティブ時は強ハイライト、非アクティブ時は弱ハイライト
    const bool l_isActivePane = a_editorWindow.GetVALActivePane() == Enum::AssetBrowserActivePaneType::AssetPane;

    // DrawList取得
    // GetWindowDrawListは現在のChildWindowのDrawListを渡す
    auto* l_drawList = ImGui::GetWindowDrawList();

    if (!l_drawList) { return; }

    // カード背景(フレーム色)
    // 選択/ホバー/通常に応じて外側カードの色を決定する
    // この色が内側ボックスの周囲(上下左右)にリムとして残り
    // 選択時に青く光る
    DrawCardBackground(l_cardMIN,
                       l_cardMAX, 
                       l_isSelected,
                       l_isHovered,
                       l_isActivePane,
                       l_isCutTarget,
                       *l_drawList);

    // アイコン領域 + アイコン
    DrawCardIcon(a_editorWindow,
                 a_filePath, 
                 l_cardMIN,
                 l_cardMAX,
                 l_isCutTarget,
                 *l_drawList);

    // ファイル名
    DrawCardFileName(a_filePath,
                     l_cardMIN,
                     l_cardMAX,
                     l_isRenaming,
                     l_isCutTarget,
                     *l_drawList);

    // 遅延ツールチップ
    // カード上にマウスが一定時間(1秒)あるとファイルパスを表示
    // IMGUIDelayedTooltipは内部でHoverDelayNormalを一時変更し
    // 指定秒数ホバー後にTooltipを表示する
    if (l_isHovered &&
        !l_isRenaming)
    {
        // ファイルパスをgeneric_stringで表示(スラッシュ区切り)
        const auto l_tooltipText = a_filePath.generic_string();

        Utility::IMGUIDelayedTooltip(l_tooltipText);
    }

    // 左クリック     : 選択
    // ダブルクリック : フォルダナビゲート
    // 右クリック     : 選択 + コンテキストメニューOpenPopup
    HandleCardClick(a_displayedFilePathList, 
                    a_filePath, 
                    l_isSelected,
                    a_editorWindow);

    // フォルダがファイル化でコンテキスト種別を切り替え
    // OnFolder : 新規フォルダ作成 + 操作系メニュー
    // OnFile   : 操作系メニューのみ
    // ポップアップDrawは毎フレーム呼ぶ(OpenPopupされていなければ内部でreturn)
    std::error_code l_errorCode = {};

    const auto l_contextType      = std::filesystem::is_directory(a_filePath, l_errorCode) ? Enum::AssetBrowserPopupContextType::AssetPane_OnFolder : Enum::AssetBrowserPopupContextType::AssetPane_OnFile;
    const auto l_contextMenuLabel = std::string{ k_cardContextMenuPrefixLabel } + a_filePath.generic_string();

    l_popupDrawer.Draw(l_selectedFilePathList,
                       a_filePath,
                       l_contextMenuLabel,
                       l_contextType,
                       a_editorWindow);
    
    // ドラッグアンドドロップ処理
    // FolderAの中にFolderBを入れるときなどに使用
    HandleCardDragDrop(a_filePath, a_editorWindow);

    // リネームモード
    // カードのファイル名領域にInputTextを重ねて描画
    if (l_isRenaming)
    {
        DrawCardRename(a_filePath,
                       l_cardMIN,
                       l_cardMAX,
                       a_editorWindow);
    }
}
void FWK::Editor::AssetBrowserEditorWindowAssetPane::DrawCardBackground(const ImVec2&     a_cardMIN, 
                                                                        const ImVec2&     a_cardMAX, 
                                                                        const bool        a_isSelected,
                                                                        const bool        a_isHovered, 
                                                                        const bool        a_isActivePane, 
                                                                        const bool        a_isCutTarget, 
                                                                              ImDrawList& a_drawList)
{
    // 外側カードをフレーム色で塗りつぶす
    // この色が内側ボックスの周囲(上下左右)にリムとして残り
    // 選択時に青く光る
    // 色の優先基準 : 切り取り対象 > 選択中(AssetPaneがアクティブ) > 選択中(AssetPaneが非アクティブ) > ホバー > 通常
    auto l_frameColor = ImGui::GetColorU32(k_cardDefaultGrayColor);

    if (a_isCutTarget)
    {
        l_frameColor = ImGui::GetColorU32(Constant::k_imguiDarkBlueTranslucentColor);
    }
    else if (a_isSelected &&
             a_isActivePane)
    {
        l_frameColor = ImGui::GetColorU32(Constant::k_imguiStrongBlueColor);
    }
    else if (a_isSelected)
    {
        l_frameColor = ImGui::GetColorU32(Constant::k_imguiStrongBlueTranslucentColor);
    }
    else if (a_isHovered)
    {
        l_frameColor = ImGui::GetColorU32(Constant::k_imguiLightGrayColor);
    }

    // 外側カードは全角丸で描画
    a_drawList.AddRectFilled(a_cardMIN,
                             a_cardMAX,
                             l_frameColor,
                             k_cardRounding,
                             ImDrawFlags_RoundCornersAll);
}
void FWK::Editor::AssetBrowserEditorWindowAssetPane::DrawCardIcon(const AssetBrowserEditorWindow& a_editorWindow,
                                                                  const std::filesystem::path&    a_filePath, 
                                                                  const ImVec2&                   a_cardMIN, 
                                                                  const ImVec2&                   a_cardMAX,
                                                                  const bool                      a_isCutTarget, 
                                                                        ImDrawList&               a_drawList) const
{
    // 上半分の内側角丸ボックスの描画
    // 外側カード(フレーム色)の上に
    // 少し縮んだ角丸ボックスをペイン背景色で描画する
    // これにより外周(上下左右)にフレームのリムが残る
    // 選択時は子のリムが青く光る
    // 上半分の高さ
    const float l_halfHeight = (a_cardMAX.y - a_cardMIN.y) * Constant::k_halfMagnification;

    // 内側ボックスの領域
    // カード外側からk_cardInnerInset分縮める(上・左・→)
    // 下辺は中間地点そのまま(下半分ボックスとぴったり接続)
    // これにより中央に区切り線が入らない
    const ImVec2& l_iconBoxMIN = { a_cardMIN.x + k_cardInnerInset, a_cardMIN.y + k_cardInnerInset };
    const ImVec2& l_iconBoxMAX = { a_cardMAX.x - k_cardInnerInset, a_cardMIN.y + l_halfHeight };

    // 内側ボックスをペイン時景色で塗りつぶし
    // これがアイコン領域の背景になる
    const auto l_paneBGColor = ImGui::GetColorU32(ImGuiCol_ChildBg);

    a_drawList.AddRectFilled(l_iconBoxMIN,
                             l_iconBoxMAX,
                             l_paneBGColor,
                             k_cardRounding,
                             ImDrawFlags_RoundCornersTop);

    // アイコン描画
    // ファイル種別に応じたアイコンを内側ボックスいっぱいに描画
    const auto& l_icon = FetchIcon(a_editorWindow, a_filePath);

    // アイコン領域のサイズ(内側ボックスから余白を引く)
    const float l_iconAreaWidth  = l_iconBoxMAX.x - l_iconBoxMIN.x - k_iconMargin * k_doubleMagnification;
    const float l_iconAreaHeight = l_iconBoxMAX.y - l_iconBoxMIN.y - k_iconMargin * k_doubleMagnification;

    // アイコンのフォントサイズを領域サイズから動的に計算
    // FontAwesomeアイコンは一文字のグリフなので
    // フォントサイズ ≒ グリフの高さとして扱える
    // 領域の幅・高さの小さい方に合わせることで
    // 縦横どちらも領域に収まる
    float l_iconFontSize = (l_iconAreaWidth < l_iconAreaHeight) ? l_iconAreaWidth : l_iconAreaHeight;

    // 領域サイズでテキストサイズを計算
    // CalcTextSize(text_begin,
    //              text_end,
    //              hide_text_after_double_hash,
    //              wrap_width);
    // 第4引数はwrap_width(折り返し幅)でありフォントサイズではないため
    // フォントサイズはAddText時に直接指定する
    const auto& l_iconSize = ImGui::CalcTextSize(l_icon.data(), l_icon.data() + l_icon.size());

    // CalcTextSizeは現在のフォントサイズ基準で計算されるため
    // 候補フォントサイズでの実際のサイズへスケール換算する
    const float l_currentFontSize = ImGui::GetFontSize();

    // 候補フォントサイズでのアイコン幅・高さを計算
    // フォントサイズに比例してグリフサイズも変化する
    const float l_scaledIconWidth  = l_iconSize.x * (l_iconFontSize / l_currentFontSize);
    const float l_scaledIconHeight = l_iconSize.y * (l_iconFontSize / l_currentFontSize);

    // 幅が領域を超える場合は幅基準へ縮小
    if (l_scaledIconWidth > l_iconAreaWidth)
    {
        l_iconFontSize = l_iconFontSize * (l_iconAreaWidth / l_scaledIconWidth);
    }

    // 高さが領域を超えr場合は高さ基準へ縮小
    if (l_scaledIconHeight > l_iconAreaHeight)
    {
        l_iconFontSize = l_iconFontSize * (l_iconAreaHeight / l_scaledIconHeight);
    }

    // 最終的なアイコンサイズを再計算
    const float l_finaleIconWidth = l_iconSize.x * (l_iconFontSize / l_currentFontSize);
    
    // アイコンを内側ボックスの中央に配置
    const ImVec2 l_iconPosition = { l_iconBoxMIN.x + (l_iconBoxMAX.x - l_iconBoxMIN.x - l_finaleIconWidth) * Constant::k_halfMagnification,
                                    l_iconBoxMIN.y + (l_iconBoxMAX.y - l_iconBoxMIN.y - l_finaleIconWidth) * Constant::k_halfMagnification };

    // アイコン色
    // 切り取り対象の場合は半透明、それ以外は通常テキスト色
    const auto l_iconColor = a_isCutTarget ? ImGui::GetColorU32(Constant::k_imguiCutTargetTextColor) : ImGui::GetColorU32(ImGuiCol_Text);

    // AddTextの第二引数に計算したフォントサイズを渡すことで
    // 指定サイズへスケール描画する
    a_drawList.AddText(ImGui::GetFont(),
                       l_iconFontSize,
                       l_iconPosition,
                       l_iconColor,
                       l_icon.data(),
                       l_icon.data() + l_icon.size());
}
void FWK::Editor::AssetBrowserEditorWindowAssetPane::DrawCardFileName(const std::filesystem::path& a_filePath, 
                                                                      const ImVec2&                a_cardMIN, 
                                                                      const ImVec2&                a_cardMAX, 
                                                                      const bool                   a_isRenaming,
                                                                      const bool                   a_isCutTarget,
                                                                            ImDrawList&            a_drawList) const
{
    // リネーム中はファイル名を描画しない
    if (a_isRenaming) { return; }

    // 名前領域(下半分の内側ボックス)
    // 上辺は中間地点そのまま(上半分ボックスとぴったり接続)
    // これにより中央に区切り線が入らない
    const float l_halfHeight = (a_cardMAX.y - a_cardMIN.y) * Constant::k_halfMagnification;

    // ファイル名取得
    // filename()でパスの最後の要素を取得
    const auto& l_fileName = a_filePath.filename().string();

    // 切り詰め可能幅 = 内側ボックス幅 - 左右余白
    const float l_maxNameWidth = (a_cardMAX.x - a_cardMIN.x) - k_cardPadding * k_doubleMagnification;

    // テキスト切り詰め
    const auto& l_displayName = TruncateText(l_fileName, l_maxNameWidth);

    // テキスト色
    // 切り取り対象の場合は半透明
    const auto& l_textColor = a_isCutTarget ? ImGui::GetColorU32(Constant::k_imguiCutTargetTextColor) : ImGui::GetColorU32(ImGuiCol_Text);

    // ファイル名を左上揃えで配置
    // 左 : 内側ボックス左端 + k_cardPadding
    // 上 : 内側ボックス上端 + k_cardPadding
    const ImVec2& l_namePosition = { a_cardMIN.x + k_cardPadding, a_cardMIN.y + l_halfHeight + k_cardPadding };

    a_drawList.AddText(l_namePosition, l_textColor, l_displayName.c_str());
}
void FWK::Editor::AssetBrowserEditorWindowAssetPane::DrawCardRename(const std::filesystem::path&    a_filePath,
                                                                    const ImVec2&                   a_cardMIN,
                                                                    const ImVec2&                   a_cardMAX,
                                                                          AssetBrowserEditorWindow& a_editorWindow) const
{
          auto& l_assetFilePathRegistry = a_editorWindow.GetMutableREFAssetFilePathRegistry();
    const auto& l_fileOperation         = a_editorWindow.GetREFFileOperation               ();
          auto& l_renameState           = a_editorWindow.GetMutableREFRenameState          ();

    // 名前領域(下半分の内側ボックス)
    // DrawCardFileNameと同じ領域を使用する
    // 上辺は中間地点そのまま(上半分ボックスとぴったり接続)
    const float l_halfHeight = (a_cardMAX.y - a_cardMIN.y) * Constant::k_halfMagnification;

    // InputTextを左上揃えで配置
    // DrawCardFileNameと同じ位置に合わせる
    const ImVec2& l_inputPosition = { a_cardMIN.x + k_cardPadding, a_cardMIN.y + l_halfHeight + k_cardPadding };

    ImGui::SetCursorScreenPos(l_inputPosition);

    // InputTextの幅を内側ボックスいっぱいにする
    const float l_inputWidth = (a_cardMAX.x - a_cardMIN.x) - k_cardPadding * k_doubleMagnification - k_cardPadding * k_doubleMagnification;

    ImGui::SetNextItemWidth(l_inputWidth);

    // 初回フレームのみフォーカス
    if (!l_renameState.m_isFocused)
    {
        ImGui::SetKeyboardFocusHere(k_keyboardFocusNextItem);
    }

    // フレームパディングを小さくしてカードになじませる
    const auto& l_style = ImGui::GetStyle();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ l_style.FramePadding.x, Constant::k_imguiInputTextHightPaddingAlignHight });

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
    const bool l_isEmptySpaceClick = ImGui::IsWindowHovered()   &&
                                     !ImGui::IsAnyItemHovered() &&
                                     ImGui::IsMouseClicked(ImGuiMouseButton_Left);

    // リネーム確定条件
    // 1.Enter押下
    // 2.フォーカス消失
    // 3.空白クリック
    // l_isEmptySpaceClickをm_isFocusedでガードしないと
    // 初回フレーム(フォーカス前)のクリックで
    // デフォルト名のまま確定してしまう
    if (l_isEnterPressed          ||
       (l_renameState.m_isFocused && 
       (!ImGui::IsItemFocused()   ||
        l_isEmptySpaceClick)))
    {
        // 先にm_isActiveをfalseにして
        // 次フレームでDrawCardRenameが呼ばれないようにする
        // これによりRenameが複数回呼ばれるのを防ぐ
        l_renameState.m_isActive  = false;
        l_renameState.m_isFocused = false;

        // InputTextの内容を取得
        // 空文字列の場合はリネームしない
        if (const auto& l_newName = std::string(l_renameState.m_inputBuffer.data());
            !l_newName.empty())
        {
            const auto& l_assetCreator = a_editorWindow.GetREFAssetCreator();

            l_fileOperation.Rename(a_filePath,
                                   l_newName,
                                   l_assetCreator,
                                   l_assetFilePathRegistry);
        }
    }

}
void FWK::Editor::AssetBrowserEditorWindowAssetPane::HandleCardClick(const std::vector<std::filesystem::path>& a_displayedFilePathList,
                                                                     const std::filesystem::path&              a_filePath, 
                                                                     const bool                                a_isSelected, 
                                                                           AssetBrowserEditorWindow&           a_editorWindow)
{
    const auto& l_popupDrawer = a_editorWindow.GetREFPopupDrawer();

    // Shift / Ctrl + クリックで範囲選択 + トグル操作
    // 修飾キーなしの場合は単一選択
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
        const auto& l_io = ImGui::GetIO();

        SelectFile(a_displayedFilePathList, 
                   a_filePath,
                   l_io.KeyShift,
                   l_io.KeyCtrl);

        // カーソル位置を更新
        m_currentCursorFilePath = a_filePath;
    }

    // フォルダカードをダブルクリックでそのフォルダへ移動
    // IsMouseDoubleClickedは画面全体でのダブルクリックを抽出するため、
    // IsItemHoverdで「このカード上で」ダブルクリックされたかを判定する
    if (std::error_code l_errorCode = {};
        ImGui::IsItemHovered()                             &&
        ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) &&
        std::filesystem::is_directory(a_filePath, l_errorCode))
    {
        NavigateToFolder(a_filePath, a_editorWindow);
    }

    // 右クリック : 選択 + コンテキストメニュー
    // 未選択のカードを右クリックした場合は選択してからメニューを開く
    // 既に選択されている場合は選択状態を維持
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
    {
        if (!a_isSelected)
        {
            SelectFile(a_displayedFilePathList, a_filePath);
        }

        // カード用コンテキストメニューを開く
        const auto& l_contextMenuLabel = std::string{ k_cardContextMenuPrefixLabel } + a_filePath.generic_string();

        l_popupDrawer.BeginPopup(l_contextMenuLabel);
    }
}

void FWK::Editor::AssetBrowserEditorWindowAssetPane::HandleCardDragDrop(const std::filesystem::path& a_filePath, AssetBrowserEditorWindow& a_editorWindow)
{
    const auto& l_selectedFilePathList = m_selectionState.GetMutableREFSelectedFilePathList();

    // D&Dドラッグ元
    // 選択中ファイル/フォルダをドラッグ
    // ルートフォルダは除外(移動するとプロジェクトが壊れるため)
    if (!l_selectedFilePathList.empty())
    {
        std::vector<std::filesystem::path> l_dragSourcePathList = {};

        for (const auto& l_selectedPath : l_selectedFilePathList)
        {
            if (l_selectedPath == Constant::k_assetRootFolderPath) { continue; }

            l_dragSourcePathList.emplace_back(l_selectedPath);
        }

        if (!l_dragSourcePathList.empty())
        {
            auto& l_imguiDragDropPayloadStorage = Utility::IMGUIDragDropPayloadStorage::GetInstance();

            // FolderPaneと同じペイロードラベルを使用することで
            // FolderPaneツリーノードへのドロップも可能にする
            l_imguiDragDropPayloadStorage.DragDropSource(Constant::k_imguiAssetBrowserFolderDragAndDropPayloadLabel, l_dragSourcePathList);
        }
    }

    // フォルダカード上にドロップされた場合、その中へ移動
    if (std::error_code l_errorCode = {};
        std::filesystem::is_directory(a_filePath, l_errorCode))
    {
        std::vector<std::filesystem::path> l_droppedFilePathList = {};

        auto& l_imguiDragDropPayloadStorage = Utility::IMGUIDragDropPayloadStorage::GetInstance();

        if (l_imguiDragDropPayloadStorage.DragDropTarget(Constant::k_imguiAssetBrowserFolderDragAndDropPayloadLabel, l_droppedFilePathList))
        {
            const auto& l_fileOperation = a_editorWindow.GetREFFileOperation();

            // ドロップされた各ファイルをこのフォルダの中へ移動
            for (const auto& l_droppedFilePath : l_droppedFilePathList)
            {
                // 自分自身へドロップした場合はスキップ
                if (l_droppedFilePath == a_filePath) { continue; }
                
                l_fileOperation.Move(l_droppedFilePath, a_filePath);
            }
        }
    }
}

void FWK::Editor::AssetBrowserEditorWindowAssetPane::BuildDisplayedFilePathList(const AssetBrowserEditorWindow& a_editorWindow, std::vector<std::filesystem::path>& a_displayedList)
{
    const auto& l_folderPane         = a_editorWindow.GetREFFolderPane             ();
    const auto& l_folderSelection    = l_folderPane.GetREFSelectionState           ();
    const auto& l_selectedFolderList = l_folderSelection.GetREFSelectedFilePathList();

    // 表示対象フォルダリストを決定
    // 複数選択時    : 選択中の全フォルダ
    // 単一/未選択時 : m_currentSelectFolderPath(空ならAssetルート)
    std::vector<std::filesystem::path> l_targetFolderList = {};

    if (l_selectedFolderList.size() > Constant::k_editorSelectedFolderSingleSize)
    {
        // 複数選択時 : 選択中フォルダをすべて表示対象にする
        l_targetFolderList = l_selectedFolderList;
    }
    else
    {
        // 単一/未選択時 : m_currentSelectFolderPath
        const auto& l_currentPath = a_editorWindow.GetREFCurrentSelectFolderPath();
        const auto& l_targetPath  = l_currentPath.empty() ? Constant::k_assetRootFolderPath : l_currentPath;

        l_targetFolderList.emplace_back(l_targetPath);
    }

    // 各フォルダの中身を収集
    // フォルダとファイルの両方を取得
    // 重複パスは発生しない(各パスは一意)
    std::error_code l_errorCode = {};

    for (const auto& l_folderPath : l_targetFolderList)
    {
        if (!std::filesystem::exists(l_folderPath, l_errorCode)) { continue; }

        for (const auto& l_entry : std::filesystem::directory_iterator(l_folderPath, l_errorCode))
        {
            a_displayedList.emplace_back(l_entry.path());
        }
    }

    // 表示順を安定させるためソート
    std::ranges::sort(a_displayedList,
                      [](const auto& a_lhs, const auto& a_rhs)
                      {
                                std::error_code l_errorCode      = {};
                          const bool            l_lhsIsDirectory = std::filesystem::is_directory(a_lhs, l_errorCode);
                          const bool            l_rhsIsDirectory = std::filesystem::is_directory(a_rhs, l_errorCode);

                          // フォルダを先にする
                          if (l_lhsIsDirectory != l_rhsIsDirectory) { return l_lhsIsDirectory; }

                          const auto& l_lhsName = a_lhs.filename().string();
                          const auto& l_rhsName = a_rhs.filename().string();

                          // 同じ種別ならファイル名で辞書順比較(大文字小文字無視)
                          return std::ranges::lexicographical_compare(l_lhsName, l_rhsName, 
                                                                      [](const char a_lhsChar, const char a_rhsChar)
                                                                      {
                                                                          return std::tolower(static_cast<unsigned char>(a_lhsChar)) < 
                                                                                 std::tolower(static_cast<unsigned char>(a_rhsChar));
                                                                      });
                      });
}

void FWK::Editor::AssetBrowserEditorWindowAssetPane::SelectFile(const std::vector<std::filesystem::path>& a_displayedFilePathList, 
                                                                const std::filesystem::path&              a_filePath,
                                                                const bool                                a_isRangeSelection, 
                                                                const bool                                a_isToggleSelection)
{
    const auto& l_rangeSelectionStartPath = m_selectionState.GetREFRangeSelectionStartPath();

    // Shift + Click : 範囲選択
    // アンカーからクリック位置までを線形順序で選択
    if (a_isRangeSelection)
    {
        // アンカー未設定の場合は単一選択として扱う
        if (l_rangeSelectionStartPath.empty())
        {
            m_selectionState.ClearSelectedFilePathList ();
            m_selectionState.AddSelectedFilePath       (a_filePath);
            m_selectionState.SetRangeSelectionStartPath(a_filePath);
        }
        else
        {
            // アンカーとクリック位置のリスト内インデックス検索
            auto l_startITR = std::find(a_displayedFilePathList.begin(), a_displayedFilePathList.end(), l_rangeSelectionStartPath);
            auto l_endITR   = std::find(a_displayedFilePathList.begin(), a_displayedFilePathList.end(), a_filePath);

            // 両方がリストに存在する場合のみ範囲選択
            if (l_startITR != a_displayedFilePathList.end() &&
                l_endITR   != a_displayedFilePathList.end())
            {
                // 開始位置がクリック位置より後ろの場合は入れ替え
                if (l_startITR > l_endITR)
                {
                    std::swap(l_startITR, l_endITR);
                }

                // アンカーからクリック位置まで(両端を含む)を選択
                m_selectionState.ClearSelectedFilePathList();

                for (auto l_itr = l_startITR; l_itr <= l_endITR; ++l_itr)
                {
                    m_selectionState.AddSelectedFilePath(*l_itr);
                }
            }
            else
            {
                // リストに存在しない場合はフォールバック
                m_selectionState.ClearSelectedFilePathList();
                m_selectionState.AddSelectedFilePath      (l_rangeSelectionStartPath);
                m_selectionState.AddSelectedFilePath      (a_filePath);
            }
        }

        return;
    }

    // Ctrl + Click : 選択/選択解除のトグル
    if (a_isToggleSelection)
    {
        const auto& l_selectedFilePathList = m_selectionState.GetREFSelectedFilePathList();

        if (auto l_itr = std::find(l_selectedFilePathList.begin(), l_selectedFilePathList.end(), a_filePath);
            l_itr != l_selectedFilePathList.end())
        {
            // 既に選択されている場合は選択解除
            m_selectionState.EraseSelectedFilePath(l_itr);
        }
        else
        {
            // 選択されてない場合は選択に追加
            m_selectionState.AddSelectedFilePath(a_filePath);
        }

        return;
    }

    // 通常クリック : 選択をクリアして単一選択
    m_selectionState.ClearSelectedFilePathList();
    m_selectionState.AddSelectedFilePath      (a_filePath);

    // 範囲選択の開始地点を更新
    m_selectionState.SetRangeSelectionStartPath(a_filePath);
}

void FWK::Editor::AssetBrowserEditorWindowAssetPane::NavigateToFolder(const std::filesystem::path& a_folderPath, AssetBrowserEditorWindow& a_editorWindow)
{
    auto& l_folderPane = a_editorWindow.GetMutableREFFolderPane();

    // フォルダツリーで親階層をすべて展開
    // これを行わないと親ノードが閉じたままで
    // このフォルダがツリーに表示されない
    auto l_parentPath = a_folderPath.parent_path();

    while (!l_parentPath.empty())
    {
        // AddFolderOpenState   : 未登録なら追加(true = 開く)
        // ApplyFolderOpenState : 既存エントリを開く
        // 両方呼ぶことで確実に開状態にする
        l_folderPane.AddFolderOpenState  (l_parentPath, true);
        l_folderPane.ApplyFolderOpenState(l_parentPath, true);

        // Assetルートに到達したら終了
        if (l_parentPath == Constant::k_assetRootFolderPath) { break; }

        l_parentPath = l_parentPath.parent_path();
    }

    // FolderPaneでこのフォルダを単一選択
    auto& l_selectionState = l_folderPane.GetMutableREFSelectionState();

    l_selectionState.SelectSingleFolder(a_folderPath, a_editorWindow);

    // AssetPaneの選択状態をクリア
    // 新しいフォルダの内容が表示されるため
    m_selectionState.ClearSelection();
    m_currentCursorFilePath.clear  ();
}

std::string FWK::Editor::AssetBrowserEditorWindowAssetPane::TruncateText(const std::string& a_text, const float a_maxWidth) const
{
    // テキスト幅が最大幅に収まる場合はそのまま消す
    if (ImGui::CalcTextSize(a_text.c_str()).x <= a_maxWidth)
    {
        return a_text;
    }

    // "..."の幅を引いた残り幅に収まる文字数を返す
    const float l_ellipsisWidth  = ImGui::CalcTextSize(k_ellipsis.data()).x;
    const float l_availableWidth = a_maxWidth - l_ellipsisWidth;

    std::string l_result = {};

    for (char l_char : a_text)
    {
        l_result += l_char;

        if (ImGui::CalcTextSize(l_result.c_str()).x > l_availableWidth)
        {
            // 1文字追加して超えたら、その文字を削って"..."をつける
            l_result.pop_back();

            break;
        }
    }

    return l_result + std::string{ k_ellipsis };
}

std::uint32_t FWK::Editor::AssetBrowserEditorWindowAssetPane::CalculateCardPerRow(const float a_availableWidth) const
{
    // 1行に並ぶカード数 = 利用可能はあ / (カード幅 + 余白)
    // 最低一枚は保証
    const float l_cardFootprint = k_cardWidth + k_cardSpacing;
          auto  l_cardsPerRow   = static_cast<std::uint32_t>(a_availableWidth / l_cardFootprint);

    if (l_cardsPerRow < k_minGuaranteeCardPerRowNUM)
    {
        l_cardsPerRow = k_minGuaranteeCardPerRowNUM;
    }

    return l_cardsPerRow;
}

std::string FWK::Editor::AssetBrowserEditorWindowAssetPane::FetchIcon(const AssetBrowserEditorWindow& a_editorWindow, const std::filesystem::path& a_filePath) const
{
    // フォルダかどうか真っ先に比較
    if (std::error_code l_errorCode = {};
        std::filesystem::is_directory(a_filePath, l_errorCode))
    {
        return std::string{ Constant::k_imguiFontAwesomeFolderCloseIcon };
    }

    const auto& l_extension = a_filePath.extension();

    if (l_extension.empty()) { return std::string{}; }

    if      (l_extension == Constant::k_lowerFBXExtension)  { return std::string{ k_imguiFontAwesomeFBXModelIcon }; }
    else if (l_extension == Constant::k_lowerPNGExtension)  { return std::string{ k_imguiFontAwesomeImageIcon }; }
    else if (l_extension == Constant::k_lowerWAVExtension)  { return std::string{ k_imguiFontAwesomeAudioIcon }; }

    // jsonファイルの場合シーンファイルなのかプレハブファイルなのかで
    // 表示するアイコンが変わるためEditor側のAssetRegistryに登録されている
    // 者のみアイコンを描画する
    const auto& l_registry = a_editorWindow.GetREFAssetFilePathRegistry();
    const auto* l_uuid     = l_registry.FindPTRAssetUUID               (a_filePath);

    if (!l_uuid) { return std::string{ k_imguiFontAwesomeFileIcon }; }

    const auto* l_assetFilePathData = l_registry.FindPTRAssetFilePathData(*l_uuid);

    if (!l_assetFilePathData) { return std::string{ k_imguiFontAwesomeFileIcon }; }

    switch (l_assetFilePathData->m_type)
    {
        case Enum::AssetFilePathRegistryType::Prefab:
        {
            return std::string{ Constant::k_imguiFontAwesomePrefabIcon };
        }
        break;

        case Enum::AssetFilePathRegistryType::Scene:
        {
            return std::string{ Constant::k_imguiFontAwesomeSceneIcon };
        }
        break;

        default:
        {
            return std::string{ k_imguiFontAwesomeFileIcon };
        }
        break;
    }
}