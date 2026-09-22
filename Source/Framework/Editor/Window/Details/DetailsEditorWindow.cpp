#include "DetailsEditorWindow.h"

void FWK::Editor::DetailsEditorWindow::Draw(EditorManager& a_editorManager)
{
    if (!ImGui::Begin(k_editorName.data()))
    {
        ImGui::End();

        return;
    }

    ReportActiveWindowIfMouseClicked(a_editorManager);

    Utility::IMGUIDelayedTooltip(k_thisWindowExplanationLabel);

    ImGui::End();
}