#pragma once


namespace FWK::Editor
{
    class WorldOutlinerEditorWindow;
}

namespace FWK::Editor
{
    class WorldOutlinerEditorWindowShortcutHandler final
    {
    public:

         WorldOutlinerEditorWindowShortcutHandler() = default;
        ~WorldOutlinerEditorWindowShortcutHandler() = default;

        void Handle(WorldOutlinerEditorWindow& a_editorWindow, EditorManager& a_editorManager) const;

    private:

        void HandleSelectAllGameObject(WorldOutlinerEditorWindow& a_editorWindow, EditorManager& a_editorManager) const;
        void HandleArrowKey           (const bool a_isRangeSelection, WorldOutlinerEditorWindow& a_editorWindow, EditorManager& a_editorManager) const;
        void HandleRename             (WorldOutlinerEditorWindow& a_editorWindow, EditorManager& a_editorManager) const;
        void HandleDeleteGameObject   (WorldOutlinerEditorWindow& a_editorWindow, EditorManager& a_editorManager) const;

        void HandleCopy     (WorldOutlinerEditorWindow& a_editorWindow, EditorManager& a_editorManager) const;
        void HandleCut      (WorldOutlinerEditorWindow& a_editorWindow, EditorManager& a_editorManager) const;
        void HandleDuplicate(WorldOutlinerEditorWindow& a_editorWindow, EditorManager& a_editorManager) const;
        void HandlePaste    (WorldOutlinerEditorWindow& a_editorWindow, EditorManager& a_editorManager) const;
    };
}