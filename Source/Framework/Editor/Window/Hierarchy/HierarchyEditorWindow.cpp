#include "HierarchyEditorWindow.h"

void FWK::Editor::HierarchyEditorWindow::Draw()
{
	// Hierarchy用のImGuiウィンドウを開始する
	if (!ImGui::Begin(k_editorName.data()))
	{
		ImGui::End();
		return;
	}

	ImGui::End();
}