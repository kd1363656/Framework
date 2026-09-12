
#include "AssetBrowserEditorWindow.h"

void FWK::Editor::AssetBrowserEditorWindow::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}
void FWK::Editor::AssetBrowserEditorWindow::PostDeserialize()
{

}

void FWK::Editor::AssetBrowserEditorWindow::Draw()
{
    if (!ImGui::Begin(k_editorName.data()))
    {
        ImGui::End();

        return;
    }

    Utility::IMGUIDelayedTooltip(k_thisWindowExplanationLabel);

    // GetContentRegionAvail()は、現在Cursor位置から
    // Window右下までに残っている描画可能領域をPixel単位で返す
    const auto& l_availableContentRegion = ImGui::GetContentRegionAvail();

    // WindowそのものをResizeした場合、
    // 現在のこのFolderPane側ではAssetPaneが最低幅を維持できない可能性がある
    // FolderPaneを描画する前にこの現在幅を有効範囲へ補正する
    m_paneSplitter.PreparePaneSize(l_availableContentRegion);

    const float l_primaryPaneSize = m_paneSplitter.GetVALPrimaryPaneSize();

    m_folderPane.Draw(l_primaryPaneSize);

    // FolderPaneの右側へSplitterを配置する
    m_paneSplitter.Draw(k_paneSplitterLabel, l_availableContentRegion);

    m_assetPane.Draw ();

    ImGui::End();
}

nlohmann::json FWK::Editor::AssetBrowserEditorWindow::Serialize()
{
    return m_jsonConverter.Serialize(*this);
}