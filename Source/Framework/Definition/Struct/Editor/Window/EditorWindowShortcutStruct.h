#pragma once

namespace FWK::Struct
{
    // ショートカット1つ分の「キーの組み合わせ」と「起きる操作」の対応
    struct EditorWindowShortcutKeyBinding final
    {
        ImGuiKeyChord m_keyChord = ImGuiKey_None;

        Enum::EditorWindowShortcutType m_shortcutType = Enum::EditorWindowShortcutType::None;
    };
}