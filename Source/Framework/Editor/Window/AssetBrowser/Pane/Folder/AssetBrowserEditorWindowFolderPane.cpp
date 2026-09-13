#include "AssetBrowserEditorWindowFolderPane.h"

void FWK::Editor::AssetBrowserEditorWindowFolderPane::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }
}

void FWK::Editor::AssetBrowserEditorWindowFolderPane::Draw(const AssetBrowserEditorWindowPopupDrawer& a_popupDrawer,
    const AssetBrowserEditorWindowAssetCreator&        a_assetCreator,
    const std::filesystem::path&                       a_assetRootFolderPath, 
    const Enum::AssetBrowserActivePaneType             a_activePane,
    const float                                        a_paneWidth, 
          AssetBrowserEditorWindowFileOperation&       a_fileOperation, 
          AssetBrowserEditorWindowClipboard&           a_clipboard, 
          AssetFilePathRegistry&                       a_assetFilePathRegistry,
          Struct::AssetBrowserEditorWindowRenameState& a_renameState)
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

    ImGui::TextUnformatted(k_paneTitleLabel.data());
    ImGui::Separator      ();

    ImGui::EndChild();
}

void FWK::Editor::AssetBrowserEditorWindowFolderPane::ClearSelection()
{

}

nlohmann::json FWK::Editor::AssetBrowserEditorWindowFolderPane::Serialize() const
{
    return nlohmann::json();
}

void FWK::Editor::AssetBrowserEditorWindowFolderPane::DrawTreeNode(const AssetBrowserEditorWindowPopupDrawer&         a_popupDrawer,
                                                                   const AssetBrowserEditorWindowAssetCreator&        a_assetCreator, 
                                                                   const std::filesystem::path&                       a_folderPath,
                                                                   const std::filesystem::path&                       a_assetFolderPath, 
                                                                   const Enum::AssetBrowserActivePaneType             a_activePane,
                                                                   const float                                        a_paneWidth, 
                                                                         AssetBrowserEditorWindowFileOperation&       a_fileOperation, 
                                                                         AssetBrowserEditorWindowClipboard&           a_clipboard, 
                                                                         AssetFilePathRegistry&                       a_assetFilePathRegistry,
                                                                         Struct::AssetBrowserEditorWindowRenameState& a_renameState)
{

}

bool FWK::Editor::AssetBrowserEditorWindowFolderPane::IsFolderOpen(const std::filesystem::path& a_folderPath) const
{
    return false;
}

void FWK::Editor::AssetBrowserEditorWindowFolderPane::ToggleFolderOpen(const std::filesystem::path& a_folderPath)
{
}

void FWK::Editor::AssetBrowserEditorWindowFolderPane::SelectFolder(const std::filesystem::path& a_folderPath, const bool a_isRangeSelection, const bool a_isToggleSelection)
{
}