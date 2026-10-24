#pragma once

namespace FWK::Enum
{
    // EditorWindow(AssetBrowser、WorldOutlinerなど)で共通して使うショートカットの種類
    // どのキーで何が起きるかはEditorWindowShortcutUtilityConstant.hの表で決める
    enum class EditorWindowShortcutType
    {
        None,
        SelectAll,
        Copy,
        Cut,
        Paste,
        Duplicate,
        Rename,
        Delete,
        CreateFolder,
    };
}