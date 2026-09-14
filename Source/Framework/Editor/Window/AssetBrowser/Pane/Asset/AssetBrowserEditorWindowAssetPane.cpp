#include "AssetBrowserEditorWindowAssetPane.h"

void FWK::Editor::AssetBrowserEditorWindowAssetPane::Draw(const AssetBrowserEditorWindowFolderPane&          a_folderPane,
                                                          const AssetBrowserEditorWindowAssetCreator&        a_assetCreator,
                                                          const std::filesystem::path&                       a_assetRootFolderPath, 
                                                                Enum::AssetBrowserActivePaneType&            a_activePane,
                                                                AssetBrowserEditorWindowPopupDrawer&         a_popupDrawer,
                                                                AssetBrowserEditorWindowFileOperation&       a_fileOperation,
                                                                AssetBrowserEditorWindowClipboard&           a_clipboard, 
                                                                AssetFilePathRegistry&                       a_assetFilePathRegistry, 
                                                                Struct::AssetBrowserEditorWindowRenameState& a_renameState)
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
        a_activePane = Enum::AssetBrowserActivePaneType::AssetPane;
    }

    ImGui::TextUnformatted(k_paneTitleLabel.data());
    ImGui::Separator      ();

    ImGui::EndChild();
}

bool FWK::Editor::AssetBrowserEditorWindowAssetPane::IsMultiSelection(const std::vector<std::filesystem::path>& a_selectedList) const
{
    // 2件以上線t買うされていれば一括選択
    // 一括選択時は選択フォルダ全ての中点をマージ描画する
    return a_selectedList.size() > Constant::k_editorSelectedFolderSingleSize;
}