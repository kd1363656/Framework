#pragma once

namespace FWK::Editor
{
    class WorldOutlinerEditorWindow;
}

namespace FWK::Editor
{
    class WorldOutlinerEditorWindowPopupDrawer final
    {
    public:

         WorldOutlinerEditorWindowPopupDrawer() = default;
        ~WorldOutlinerEditorWindowPopupDrawer() = default;

        void BeginPopup(const std::string_view& a_openPopupLabel) const;

        void DrawScenePopup(const std::string_view&          a_popupLabel,
                                  Scene&                     a_scene,
                                  WorldOutlinerEditorWindow& a_editorWindow,
                                  EditorManager&             a_editorManager) const;

        void DrawGameObjectPopup(const std::weak_ptr<GameObject>& a_gameObject,
                                 const std::string_view&          a_popupLabel,
                                       Scene&                     a_scene,
                                       WorldOutlinerEditorWindow& a_editorWindow,
                                       EditorManager&             a_editorManager) const;

        void DrawEmptySpacePopup(const std::string_view&          a_popupLabel,
                                       Scene&                     a_scene,
                                       WorldOutlinerEditorWindow& a_editorWindow,
                                       EditorManager&             a_editorManager) const;

    private:

        void DrawCreateEmptyGameObjectMenu(const std::weak_ptr<GameObject>& a_parent,
                                                 Scene&                     a_scene,
                                                 WorldOutlinerEditorWindow& a_editorWindow,
                                                 EditorManager&             a_editorManager) const;

        static constexpr std::string_view k_createEmptyGameObjectMenuLabel = "空のゲームオブジェクトを作成";
        static constexpr std::string_view k_renameMenuLabel                = "名前変更";
        static constexpr std::string_view k_deleteMenuLabel                = "削除";

        static constexpr std::string_view k_copyMenuLabel      = "コピー";
        static constexpr std::string_view k_cutMenuLabel       = "切り取り";
        static constexpr std::string_view k_duplicateMenuLabel = "複製";
        static constexpr std::string_view k_pasteMenuLabel     = "貼り付け";

        static constexpr std::string_view k_renameShortcutLabel    = "F2";
        static constexpr std::string_view k_copyShortcutLabel      = "Ctrl + C";
        static constexpr std::string_view k_cutShortcutLabel       = "Ctrl + X";
        static constexpr std::string_view k_pasteShortcutLabel     = "Ctrl + V";
        static constexpr std::string_view k_duplicateShortcutLabel = "Ctrl + D";
        static constexpr std::string_view k_deleteShortcutLabel    = "Del";
    };
}