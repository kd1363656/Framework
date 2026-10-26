#pragma once

namespace FWK::Editor
{
    class FileMainMenuEditor final : public EditorMainMenuBase
    {
    public:

         FileMainMenuEditor()          = default;
        ~FileMainMenuEditor() override = default;

        void Draw() override;

    private:

        void DrawMenuFile             ();
        void UpdateShortCutKey        ();
        void ApplyFileDialogResultList();

        void NewScene   () const;
        void OpenScene  ();
        void SaveScene  ();
        void SaveSceneAs();

        bool CanOperateScene() const;

        Struct::FileDialogDesc CreateSceneFileDialogDesc(const std::wstring_view& a_title) const;

        static constexpr std::string_view k_beginFileMenuTextString = "ファイル";
        static constexpr std::string_view k_newSceneTextString      = "新規シーンの作成";
        static constexpr std::string_view k_openSceneTextString     = "シーンを開く";
        static constexpr std::string_view k_saveTextString          = "保存";
        static constexpr std::string_view k_saveAsTextString        = "名前を付けて保存";

        static constexpr std::string_view k_newSceneShortCutString  = "Ctrl+N";
        static constexpr std::string_view k_openSceneShortCutString = "Ctrl+O";
        static constexpr std::string_view k_saveShortCutString      = "Ctrl+S";
        static constexpr std::string_view k_saveAsShortCutString    = "Ctrl+Shift+S";

        static constexpr std::wstring_view k_openSceneFileDialogTitle = L"シーンファイルを開く";
        static constexpr std::wstring_view k_saveSceneFileDialogTitle = L"シーンファイルを保存";

        static constexpr ImGuiKeyChord k_newSceneKeyChord  = ImGuiMod_Ctrl | ImGuiKey_N;
        static constexpr ImGuiKeyChord k_openSceneKeyChord = ImGuiMod_Ctrl | ImGuiKey_O;
        static constexpr ImGuiKeyChord k_saveKeyChord      = ImGuiMod_Ctrl | ImGuiKey_S;
        static constexpr ImGuiKeyChord k_saveAsKeyChord    = ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_S;

        static constexpr bool k_isMenuItemNotSelected = false;

        Utility::FileDialog m_openSceneFileDialog = {};
        Utility::FileDialog m_saveSceneFileDialog = {};

        FWK_DEFINE_TYPE_INFO(FileMainMenuEditor, EditorMainMenuBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorMainMenuUniqueFactory, FWK::Editor::FileMainMenuEditor)