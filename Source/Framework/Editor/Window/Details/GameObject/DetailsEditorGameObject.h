#pragma once

namespace FWK::Editor
{
    class DetailsEditorGameObject final
    {
    public:

         DetailsEditorGameObject() = default;
        ~DetailsEditorGameObject() = default;

        void Draw(const std::weak_ptr<GameObject>& a_gameObject);

    private:

        void DrawAddComponentButton(const std::weak_ptr<GameObject>& a_gameObject);

        bool DrawComponentMenuButton() const;

        void RemoveComponent(const std::weak_ptr<GameObject>& a_gameObject, const std::shared_ptr<GameObjectComponentBase>& a_component) const;

        static constexpr std::string_view k_gameObjectHeaderLabel        = "ゲームオブジェクト基本情報    ";
        static constexpr std::string_view k_gameObjectNameLabel          = "名前                          ";
        static constexpr std::string_view k_prefabUUIDLabel              = "プレハブUUID                  ";
        static constexpr std::string_view k_prefabHierarchyNodeUUIDLabel = "プレハブヒエラルキーノードUUID";
        static constexpr std::string_view k_sceneInstanceUUIDLabel       = "シーンインスタンスUUID        ";
        static constexpr std::string_view k_addComponentButtonLabel      = "コンポーネント追加";
        static constexpr std::string_view k_addComponentPopupLabel       = "##DetailsEditorGameObjectAddComponentPopup";
        static constexpr std::string_view k_componentMenuButtonLabel     = "\xEF\x85\x82##DetailsEditorGameObjectComponentMenuButton";
        static constexpr std::string_view k_componentMenuPopupLabel      = "##DetailsEditorGameObjectComponentMenuPopup";
        static constexpr std::string_view k_removeMenuLabel              = "削除";

        static constexpr float k_addComponentButtonWidth  = 230.0F;
        static constexpr float k_autoFitButtonHeight      = 0.0F;
        static constexpr float k_framePaddingBothSidesNUM = 2.0F;

        DetailsEditorGameObjectAddComponentPopupDrawer m_addComponentPopupDrawer = {};
    };
}