#include "AssetBrowserEditorWindowAssetPane.h"

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
   
    ImGui::EndChild();
}