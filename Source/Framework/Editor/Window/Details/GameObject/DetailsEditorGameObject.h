#pragma once

namespace FWK::Editor
{
    class DetailsEditorGameObject final
    {
    public:

         DetailsEditorGameObject() = default;
        ~DetailsEditorGameObject() = default;

        void Draw(GameObject& a_gameObject);

    private:

        void DrawAddComponentButton(GameObject& a_gameObject);

        static constexpr std::string_view k_gameObjectHeaderLabel        = "ゲームオブジェクト基本情報    ";
        static constexpr std::string_view k_gameObjectNameLabel          = "名前                          ";
        static constexpr std::string_view k_prefabUUIDLabel              = "プレハブUUID                  ";
        static constexpr std::string_view k_prefabHierarchyNodeUUIDLabel = "プレハブヒエラルキーノードUUID";
        static constexpr std::string_view k_sceneInstanceUUIDLabel       = "シーンインスタンスUUID        ";
        static constexpr std::string_view k_addComponentButtonLabel      = "コンポーネント追加";
        static constexpr std::string_view k_addComponentPopupLabel       = "##DetailsEditorGameObjectAddComponentPopup";

        static constexpr float k_addComponentButtonWidth = 230.0F;
        static constexpr float k_autoFitButtonHeight     = 0.0F;

        DetailsEditorGameObjectAddComponentPopupDrawer m_addComponentPopupDrawer = {};
    };
}