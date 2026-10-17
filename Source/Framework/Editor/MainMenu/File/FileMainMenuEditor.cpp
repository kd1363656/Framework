#include "FileMainMenuEditor.h"

void FWK::Editor::FileMainMenuEditor::Draw()
{
    DrawMenuFile     ();
    UpdateShortCutKey();
}

void FWK::Editor::FileMainMenuEditor::DrawMenuFile()
{
    if (!ImGui::BeginMenu(k_beginFileMenuTextString.data())) { return; }

    // 上下キーとマウスで項目を選べるようにする
    m_popupNavigator.Begin(Enum::EditorPopupNavigationAxis::Vertical);

    if (m_popupNavigator.MenuItem(k_saveTextString.data(), k_saveShortCutString.data()))
    {
        // すべてのシーン情報をセーブ
        const auto& l_sceneManager = SceneManager::GetInstance();

        l_sceneManager.Save();

        FWK_ADD_LOG(Constant::k_imguiDebugSuccessColor, "シーンのセーブが完了しました。");
    }

    m_popupNavigator.End();

    ImGui::EndMenu();
}
void FWK::Editor::FileMainMenuEditor::UpdateShortCutKey() const
{
    if (ImGui::IsKeyDown(ImGuiMod_Ctrl) &&
        ImGui::IsKeyPressed(ImGuiKey_S))
    {
        // すべてのシーン情報をセーブ
        const auto& l_sceneManager = SceneManager::GetInstance();

        l_sceneManager.Save();

        FWK_ADD_LOG(Constant::k_imguiDebugSuccessColor, "シーンのセーブが完了しました。");
    }
}