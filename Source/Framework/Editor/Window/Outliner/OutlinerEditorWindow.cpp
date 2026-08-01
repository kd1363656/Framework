#include "OutlinerEditorWindow.h"

void FWK::Editor::OutlinerEditorWindow::Draw()
{
	// Hierarchy用のImGuiウィンドウを開始する
	if (!ImGui::Begin(k_editorName.data()))
	{
		ImGui::End();
		return;
	}

	ImGui::End();
}

void FWK::Editor::OutlinerEditorWindow::DrawGameObjectNode(const std::shared_ptr<GameObject>& a_gameObject)
{

}