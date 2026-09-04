#include "DetailsEditorWindow.h"

void FWK::Editor::DetailsEditorWindow::Draw()
{
	if (!ImGui::Begin(k_editorName.data()))
	{
		ImGui::End();

		return;
	}

	ImGui::End();
}