#pragma once

namespace FWK::Editor
{
    class DetailsEditorWindow final : public EditorWindowBase
    {
    public:

         DetailsEditorWindow()          = default;
        ~DetailsEditorWindow() override = default;

        void Draw() override;

    private:

        static constexpr std::string_view k_editorName                 = "詳細";
        static constexpr std::string_view k_thisWindowExplanationLabel = "各ゲームオブジェクトに割り当てられているコンポーネントやシーンのパラメータを見ることができるウィンドウ。";

        FWK_DEFINE_TYPE_INFO(DetailsEditorWindow, EditorWindowBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::DetailsEditorWindow)