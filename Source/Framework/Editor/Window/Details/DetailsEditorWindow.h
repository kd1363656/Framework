#pragma once

namespace FWK::Editor
{
    class DetailsEditorWindow final : public EditorWindowBase
    {
    public:

         DetailsEditorWindow()          = default;
        ~DetailsEditorWindow() override = default;

        void Draw(EditorManager& a_editorManager) override;

    private:

        void DrawGameObjectDetails(const FWK::GameObject& a_gameObject) const;

        static constexpr std::string_view k_editorName                   = "詳細";
        static constexpr std::string_view k_thisWindowExplanationLabel   = "各ゲームオブジェクトに割り当てられているコンポーネントやシーンのパラメータを見ることができるウィンドウ。";
        static constexpr std::string_view k_noSelectionLabel             = "ゲームオブジェクトが選択されていません。";
        static constexpr std::string_view k_gameObjectHeaderLabel        = "ゲームオブジェクト基本情報    ";
        static constexpr std::string_view k_gameObjectNameLabel          = "名前                          ";
        static constexpr std::string_view k_prefabUUIDLabel              = "プレハブUUID                  ";
        static constexpr std::string_view k_prefabHierarchyNodeUUIDLabel = "プレハブヒエラルキーノードUUID";
        static constexpr std::string_view k_sceneInstanceUUIDLabel       = "シーンインスタンスUUID        ";

        FWK_DEFINE_TYPE_INFO(DetailsEditorWindow, EditorWindowBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::DetailsEditorWindow)