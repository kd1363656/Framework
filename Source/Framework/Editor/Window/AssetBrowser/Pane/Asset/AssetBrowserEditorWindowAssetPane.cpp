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

        DrawCard(l_filePath, a_editorWindow);

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

void FWK::Editor::AssetBrowserEditorWindowAssetPane::DrawCard(const std::filesystem::path& a_filePath, AssetBrowserEditorWindow& a_editorWindow)
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