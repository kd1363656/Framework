#pragma once

namespace FWK::Constant
{
    // ショートカットの数(k_editorWindowShortcutKeyBindingListに追加・削除したら、この数も合わせる)
    inline constexpr std::size_t k_editorWindowShortcutKeyBindingCount = 8ULL;

    // 修飾キー(Ctrl、Shift、Alt)は完全一致で判定し、上から順番に最初に押されていたものを採用する
    inline constexpr std::array<Struct::EditorWindowShortcutKeyBinding, k_editorWindowShortcutKeyBindingCount> k_editorWindowShortcutKeyBindingList =
    {{
        { ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_N, Enum::EditorWindowShortcutType::CreateFolder },
        { ImGuiMod_Ctrl | ImGuiKey_A,                  Enum::EditorWindowShortcutType::SelectAll    },
        { ImGuiMod_Ctrl | ImGuiKey_C,                  Enum::EditorWindowShortcutType::Copy         },
        { ImGuiMod_Ctrl | ImGuiKey_X,                  Enum::EditorWindowShortcutType::Cut          },
        { ImGuiMod_Ctrl | ImGuiKey_V,                  Enum::EditorWindowShortcutType::Paste        },
        { ImGuiMod_Ctrl | ImGuiKey_D,                  Enum::EditorWindowShortcutType::Duplicate    },
        { ImGuiKey_F2,                                 Enum::EditorWindowShortcutType::Rename       },
        { ImGuiKey_Delete,                             Enum::EditorWindowShortcutType::Delete       },
    }};
}