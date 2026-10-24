#pragma once

namespace FWK::Utility
{
    // このフレームで押されたEditorWindow共通のショートカットを返す(何も押されていなければNone)
    // AssetBrowserとWorldOutlinerのShortcutHandlerはどちらもこの関数でキー入力を判定し、
    // 返ってきた種類に応じて各ウィンドウの処理を呼ぶ
    // (キーの判定をここにまとめることで、ウィンドウによってキー割り当てや押しっぱなし時の動きがずれないようにする)
    inline Enum::EditorWindowShortcutType FetchVALPressedEditorWindowShortcut()
    {
        for (const auto& l_keyBinding : Constant::k_editorWindowShortcutKeyBindingList)
        {
            // ImGui::IsKeyChordPressed : 修飾キーが完全に一致した状態で、キーが押された瞬間だけtrueを返す
            // 押しっぱなしでは繰り返しtrueにならないため、Ctrl + Vを押し続けても貼り付けは1回だけになる
            if (ImGui::IsKeyChordPressed(l_keyBinding.m_keyChord)) { return l_keyBinding.m_shortcutType; }
        }

        return Enum::EditorWindowShortcutType::None;
    }

    // 矢印キーで選択を移動する時に、範囲選択するかどうか
    // Shift又はCtrlを押していれば範囲選択にする(AssetBrowserとWorldOutlinerで共通)
    inline bool IsEditorWindowRangeSelectionKeyDown()
    {
        const auto& l_io = ImGui::GetIO();

        return l_io.KeyShift ||
               l_io.KeyCtrl;
    }
}