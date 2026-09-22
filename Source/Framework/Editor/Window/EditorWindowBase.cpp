#include "EditorWindowBase.h"

void FWK::Editor::EditorWindowBase::ReportActiveWindowIfMouseClicked(EditorManager& a_editorManager) const
{
    // 現在のウィンドウ上にマウスがあり
    // 左クリックまたは右クリックされた場合
    // 派生クラスのStaticTypeIDを取得しそれをActiveWindowとする
    if (ImGui::IsWindowHovered() &&
       (ImGui::IsMouseClicked(ImGuiMouseButton_Left) ||
        ImGui::IsMouseClicked(ImGuiMouseButton_Right)))
    {
        const auto& l_typeINFO = GetREFRuntimeTypeINFO();

        a_editorManager.SstCurrentActiveWindowStaticTpeID(l_typeINFO.k_staticTypeID);
    }
}