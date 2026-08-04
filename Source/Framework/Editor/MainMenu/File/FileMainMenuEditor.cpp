#include "FileMainMenuEditor.h"

void FWK::Editor::FileMainMenuEditor::Draw()
{
	DrawMenuFile     ();
	UpdateShortCutKey();
}

void FWK::Editor::FileMainMenuEditor::DrawMenuFile() const
{
	if (ImGui::BeginMenu(k_beginFileMenuTextString.data()))
	{
		if (ImGui::MenuItem(k_saveTextString.data(), k_saveShortCutString.data()))
		{
			// すべてのシーン情報をセーブ
			const auto& l_sceneManager = SceneManager::GetInstance();
			
			l_sceneManager.SaveScene();
			
			FWK_ADD_LOG("シーンのセーブが完了しました。");
		}

		ImGui::EndMenu();
	}
}
void FWK::Editor::FileMainMenuEditor::UpdateShortCutKey() const
{
	if (ImGui::IsKeyDown(ImGuiMod_Ctrl) && ImGui::IsKeyPressed(ImGuiKey_S))
	{
		// すべてのシーン情報をセーブ
		const auto& l_sceneManager = SceneManager::GetInstance();

		l_sceneManager.SaveScene();

		FWK_ADD_LOG("シーンのセーブが完了しました。");
	}
}