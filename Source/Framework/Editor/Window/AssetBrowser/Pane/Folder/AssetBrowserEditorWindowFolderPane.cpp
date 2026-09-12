#include "AssetBrowserEditorWindowFolderPane.h"

void FWK::Editor::AssetBrowserEditorWindowFolderPane::Draw(const float a_paneWidth)
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