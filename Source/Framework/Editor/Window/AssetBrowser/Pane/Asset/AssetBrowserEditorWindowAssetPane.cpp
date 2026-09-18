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
}
void FWK::Editor::AssetBrowserEditorWindowAssetPane::MoveSelectionDown(AssetBrowserEditorWindow& a_editorWindow, const bool a_isRangeSelection)
{
}
void FWK::Editor::AssetBrowserEditorWindowAssetPane::MoveSelectionLeft(AssetBrowserEditorWindow& a_editorWindow, const bool a_isRangeSelection)
{
}
void FWK::Editor::AssetBrowserEditorWindowAssetPane::MoveSelectionRight(AssetBrowserEditorWindow& a_editorWindow, const bool a_isRangeSelection)
{
}

void FWK::Editor::AssetBrowserEditorWindowAssetPane::NavigateToCurrentCursor(AssetBrowserEditorWindow& a_editorWindow)
{
}

std::vector<std::filesystem::path> FWK::Editor::AssetBrowserEditorWindowAssetPane::FetchVALDisplayedFilePathList(AssetBrowserEditorWindow& a_editorWindow)
{
    return std::vector<std::filesystem::path>();
}

std::filesystem::path FWK::Editor::AssetBrowserEditorWindowAssetPane::FetchVALOperationTargetFilePath() const
{
    return std::filesystem::path();
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

    // カード背景
    DrawCardBackground(l_cardMIN, l_cardMAX, *l_drawList);

    // アイコン領域 + アイコン
    DrawCardIcon(a_filePath, 
                 l_cardMIN,
                 l_cardMAX,
                 l_isCutTarget,
                 a_editorWindow,
                 *l_drawList);

    // ファイル名
    DrawCardFileName(a_filePath,
                     l_cardMIN,
                     l_cardMAX,
                     l_isRenaming,
                     l_isCutTarget,
                     *l_drawList);

    // ハイライト枠
    DrawCardHighlight(l_cardMIN,
                      l_cardMAX,
                      l_isSelected,
                      l_isHovered,
                      l_isActivePane,
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
void FWK::Editor::AssetBrowserEditorWindowAssetPane::DrawCardBackground(const ImVec2& a_cardMIN, const ImVec2& a_cardMAX, ImDrawList& a_drawList)
{
    // ペイン背景色(ImGuiCol_ChildBg)でカード全体を塗りつぶし角丸のあるカードにする
    const auto& l_cardBGColor = ImGui::GetColorU32(ImGuiCol_ChildBg);

    a_drawList.AddRectFilled(a_cardMIN,
                             a_cardMAX,
                             l_cardBGColor,
                             k_cardRounding,
                             ImDrawFlags_RoundCornersTop);
}
void FWK::Editor::AssetBrowserEditorWindowAssetPane::DrawCardIcon(const std::filesystem::path&    a_filePath, 
                                                                  const ImVec2&                   a_cardMIN, 
                                                                  const ImVec2&                   a_cardMAX,
                                                                  const bool                      a_isCutTarget, 
                                                                        AssetBrowserEditorWindow& a_editorWindow, 
                                                                        ImDrawList&               a_drawList) const
{
    // アイコン領域描画(上半分)
    // アセットペイン背景色と同じ色で上半分を塗りつぶす
    // 領域仕様 : アイコン周りはアセットペインの背景色と同じ色
    // カードより左右をk_iconAreaInset分縮める
    const ImVec2& l_iconAreaMIN = { a_cardMIN.x + k_iconAreaInset, a_cardMIN.y };
    const ImVec2& l_iconAreaMAX = { a_cardMAX.x - k_iconAreaInset, a_cardMIN.y + (a_cardMAX.y - a_cardMIN.y) * Constant::k_halfMagnification };

    // 上半分も背景色と同じなのdえ追加描画は省略可能だが
    // 明示的にアイコン領域を示すため描画(同色なので見た目は変わらない)
    // 将来的にアイコン領域の色を変えたい場合はここを変更する
    const auto& l_cardBGColor = ImGui::GetColorU32(ImGuiCol_ChildBg);

    a_drawList.AddRectFilled(l_iconAreaMIN,
                             l_iconAreaMAX,
                             l_cardBGColor,
                             k_cardRounding,
                             ImDrawFlags_RoundCornersTop);

    // アイコン描画
    // ファイル種別に応じたアイコンを中央配置
    const auto&   l_icon     = FetchIcon(a_filePath, a_editorWindow);
    const ImVec2& l_iconSize = ImGui::CalcTextSize(l_icon.data(), l_icon.data() + l_icon.size());

    // アイコンをアイコン霊異記の中央に配置
    const ImVec2& l_iconPosition = { l_iconAreaMIN.x + (l_iconAreaMAX.x - l_iconAreaMIN.x - l_iconSize.x) * Constant::k_halfMagnification,
                                     l_iconAreaMIN.y + (l_iconAreaMAX.y - l_iconAreaMIN.y - l_iconSize.y) * Constant::k_halfMagnification };

    // アイコン色
    // 切り取り対象の場合は半透明、それ以外は通常テキスト色
    const auto& l_iconColor = a_isCutTarget ? ImGui::GetColorU32(Constant::k_imguiCutTargetTextColor) : ImGui::GetColorU32(ImGuiCol_Text);

    a_drawList.AddText(ImGui::GetFont(),
                       ImGui::GetFontSize(),
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
    // DrawCardRenameでInputTextTextが描画されるため
    if (a_isRenaming) { return; }

    // 名前領域
    const ImVec2& l_nameAreaMIN = { a_cardMIN.x, a_cardMIN.y + (a_cardMAX.y - a_cardMIN.y) * Constant::k_halfMagnification };
    const ImVec2& l_nameAreaMAX = a_cardMAX;

    // ファイル名取得
    // filename()でパスの最後の要素を取得
    const auto& l_fileName = a_filePath.filename().string();

    // 切り詰め可能幅 = カード幅 - 左右余白
    const float l_maxNameWidth = (a_cardMAX.x - a_cardMIN.x) - k_cardPadding * k_doubleMagnification;

    // テキスト切り詰め
    const auto& l_displayName = TruncateText(l_fileName, l_maxNameWidth);

    // テキスト色
    // 切り取り対象の場合は半透明
    const auto& l_textColor = a_isCutTarget ? ImGui::GetColorU32(Constant::k_imguiCutTargetTextColor) : ImGui::GetColorU32(ImGuiCol_Text);

    // ファイル名を名前領域の中央に配置
    const auto&   l_nameSize     = ImGui::CalcTextSize(l_displayName.c_str());
    const ImVec2& l_namePosition = { l_nameAreaMIN.x + (l_nameAreaMAX.x - l_nameAreaMIN.x - l_nameSize.x) * Constant::k_halfMagnification,
                                   l_nameAreaMIN.y + (l_nameAreaMAX.y - l_nameAreaMIN.y - l_nameSize.y) * Constant::k_halfMagnification };

    a_drawList.AddText(l_namePosition, l_textColor, l_displayName.c_str());
}
void FWK::Editor::AssetBrowserEditorWindowAssetPane::DrawCardHighlight(const ImVec2&     a_cardMIN,
                                                                       const ImVec2&     a_cardMAX,
                                                                       const bool        a_isSelected, 
                                                                       const bool        a_isHovered,
                                                                       const bool        a_isActivePane, 
                                                                       const bool        a_isCutTarget, 
                                                                             ImDrawList& a_drawList) const
{
    // 選択中   : 強い青(アクティブ) / 半透明青(非アクティブ) / 半透明(切り取り対象)
    // ホバー中 : 明るいグレー
    // それ以外 : 枠なし
    if (a_isSelected)
    {
        auto l_selectionColor = k_initialSelectionColor;

        // 選択中の枠色を決定
        // 切り取り対象 > アクティブペイン > 非アクティブペインの優先順位
        if (a_isCutTarget)
        {
            l_selectionColor = ImGui::GetColorU32(Constant::k_imguiDarkBlueTranslucentColor);
        }
        else if (a_isActivePane)
        {
            l_selectionColor = ImGui::GetColorU32(Constant::k_imguiStrongBlueColor);
        }
        else
        {
            l_selectionColor = ImGui::GetColorU32(Constant::k_imguiStrongBlueTranslucentColor);
        }

        // 角丸枠線を描画
        a_drawList.AddRect(a_cardMIN,
                           a_cardMAX,
                           l_selectionColor,
                           ImDrawFlags_RoundCornersAll,
                           k_borderThickness);
    }
    else if (a_isHovered)
    {
        // ホバー時は明るいグレーの枠
        // k_imguiLightGrayColorはFolderPaneでも使う共通定数
        const auto& l_hoverColor = ImGui::GetColorU32(Constant::k_imguiLightGrayColor);

        a_drawList.AddRect(a_cardMIN,
                           a_cardMAX,
                           l_hoverColor,
                           ImDrawFlags_RoundCornersAll,
                           k_borderThickness);
    }

}
void FWK::Editor::AssetBrowserEditorWindowAssetPane::HandleCardClick(const std::vector<std::filesystem::path>& a_displayedFilePathList,
                                                                     const std::filesystem::path&              a_filePath, 
                                                                     const bool                                a_isSelected, 
                                                                           AssetBrowserEditorWindow&           a_editorWindow)
{
    
}

void FWK::Editor::AssetBrowserEditorWindowAssetPane::BuildDisplayedFilePathList(AssetBrowserEditorWindow& a_editorWindow, std::vector<std::filesystem::path>& a_displayedList)
{

}

void FWK::Editor::AssetBrowserEditorWindowAssetPane::SelectFile(const std::vector<std::filesystem::path>& a_displayedFilePathList, 
                                                                const std::filesystem::path&              a_filePath,
                                                                     AssetBrowserEditorWindow&            a_editorWindow, 
                                                                const bool                                a_isRangeSelection, 
                                                                const bool                                a_isToggleSelection)
{

}