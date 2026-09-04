#include "WorldOutlinerEditorWindow.h"

void FWK::Editor::WorldOutlinerEditorWindow::Draw()
{
	// Outliner用ImGuiウィンドウを開始
	if (!ImGui::Begin(k_editorName.data()))
	{
		ImGui::End();

		return;
	}

	ImGui::End();
}