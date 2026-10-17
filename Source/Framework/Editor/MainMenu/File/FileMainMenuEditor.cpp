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
        // 保存と「*」を消す処理は終了確認ダイアログと同じものを使う
        auto& l_editorManager = EditorManager::GetInstance();

        l_editorManager.SaveScene();
    }

    m_popupNavigator.End();

    ImGui::EndMenu();
}
void FWK::Editor::FileMainMenuEditor::UpdateShortCutKey() const
{
    if (ImGui::IsKeyDown(ImGuiMod_Ctrl) &&
        ImGui::IsKeyPressed(ImGuiKey_S))
    {
        auto& l_editorManager = EditorManager::GetInstance();

        l_editorManager.SaveScene();
    }
}