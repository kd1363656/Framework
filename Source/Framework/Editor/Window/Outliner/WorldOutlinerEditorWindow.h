#pragma once

namespace FWK::Editor
{
    class AssetBrowserEditorWindow;
}

namespace FWK::Editor
{
    class WorldOutlinerEditorWindow final : public EditorWindowBase
    {
    public:

         WorldOutlinerEditorWindow()          = default;
        ~WorldOutlinerEditorWindow() override = default;

        void Draw(EditorManager& a_editorManager) override;

    private:
       
        static constexpr std::string_view k_editorName                 = "アウトライナー";
        static constexpr std::string_view k_emptySceneLabel            = "UntitledScene";
        static constexpr std::string_view k_emptyAreaLabel             = "##WorldOutlinerEmptyArea";
        static constexpr std::string_view k_thisWindowExplanationLabel = "アウトライナーでは現在読み込んでいるシーン、シーンに含まれるゲームオブジェクトを見ることができ\n親子関係を結ぶ、名前を変える、シーンからゲームオブジェクトを削除することができるウィンドウ。";
        static constexpr std::string_view k_noCurrentSceneLabel        = "現在読み込まれているシーンはありません。";

        static constexpr float k_minEmptyAreaSize = 0.0F;

        WorldOutlinerEditorWindowSelectionState m_selectionState = {};

        FWK_DEFINE_TYPE_INFO(WorldOutlinerEditorWindow, EditorWindowBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::WorldOutlinerEditorWindow)