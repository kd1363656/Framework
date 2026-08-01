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