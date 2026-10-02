#include "EditorWindowBase.h"

void FWK::Editor::EditorWindowBase::ReportActiveWindowIfMouseClicked(EditorManager& a_editorManager) const
{
    // 現在のウィンドウ上にマウスがあり
    // 左クリックまたは右クリックされた場合
    // 派生クラスのStaticTypeIDを取得しそれをActiveWindowとする
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
    {
        const auto& l_typeINFO = GetREFRuntimeTypeINFO();

        a_editorManager.SetCurrentActiveWindowStaticTpeID(l_typeINFO.k_staticTypeID);
    }
}