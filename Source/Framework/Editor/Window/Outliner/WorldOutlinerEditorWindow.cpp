#include "WorldOutlinerEditorWindow.h"

void FWK::Editor::WorldOutlinerEditorWindow::Draw()
{
	// Outliner用ImGuiウィンドウを開始
	if (!ImGui::Begin(k_editorName.data()))
	{
		ImGui::End();

		return;
	}

	Utility::IMGUIDelayedTooltip(k_thisWindowExplanationLabel);

	// SceneManagerが現在所有しているSceneをweak_ptrから取得する
	// lock()したshared_ptrはこのDraw()の間だけSceneの生存を保証する
	const auto& l_sceneManager = SceneManager::GetInstance ();
	const auto& l_scene        = l_sceneManager.GetVALScene().lock();

	if (!l_scene)
	{
		// TextDisabled()は通常のText()より薄い色で文字を描画する
		// Scene未読み込みはエラーではないため、警告色ではなく補助表示にする
		ImGui::TextDisabled(k_noCurrentSceneLabel.data());

		ImGui::End();

		return;
	}

	DrawSceneNode(*l_scene);

	ImGui::End();
}

void FWK::Editor::WorldOutlinerEditorWindow::DrawSceneNode(const Scene& a_scene) const
{

}
void FWK::Editor::WorldOutlinerEditorWindow::DrawGameObjectNode(const GameObject& a_gameObject) const
{

}