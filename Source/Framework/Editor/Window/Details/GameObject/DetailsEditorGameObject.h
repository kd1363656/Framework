#pragma once

namespace FWK::Editor
{
    class DetailsEditorGameObject final
    {
    public:

         DetailsEditorGameObject() = default;
        ~DetailsEditorGameObject() = default;

        void Draw(const FWK::GameObject& a_gameObject);

    private:

        void DrawTransformComponent(const FWK::GameObject& a_gameObject);
        void DrawComponentList     (const FWK::GameObject& a_gameObject);
        void DrawAddComponentButton(const FWK::GameObject& a_gameObject);

        void DrawComponent(const std::shared_ptr<GameObjectComponentBase>& a_component,
                                 const FWK::GameObject&                          a_gameObject,
                           std::weak_ptr<GameObjectComponentBase>&   a_removeComponent);

        void AddComponent(const std::string_view& a_typeName, const FWK::GameObject& a_gameObject) const;

        bool CanAddComponent(const std::string_view& a_typeName, const FWK::GameObject& a_gameObject) const;

        std::vector<Struct::TaggedFactoryTagGroup> FetchVALAddComponentTagGroupList(const FWK::GameObject& a_gameObject) const;

        static constexpr std::string_view k_gameObjectHeaderLabel        = "ゲームオブジェクト基本情報    ";
        static constexpr std::string_view k_gameObjectNameLabel          = "名前                          ";
        static constexpr std::string_view k_prefabUUIDLabel              = "プレハブUUID                  ";
        static constexpr std::string_view k_prefabHierarchyNodeUUIDLabel = "プレハブヒエラルキーノードUUID";
        static constexpr std::string_view k_sceneInstanceUUIDLabel       = "シーンインスタンスUUID        ";

        static constexpr std::string_view k_componentEnableLabel      = "有効";
        static constexpr std::string_view k_componentContextMenuLabel = "##DetailsComponentContextMenu";
        static constexpr std::string_view k_removeComponentLabel      = "コンポーネントを削除";
        static constexpr std::string_view k_addComponentButtonLabel   = "コンポーネントを追加";
        static constexpr std::string_view k_addComponentPopupLabel    = "##DetailsAddComponentPopup";

        static constexpr ImVec2 k_addComponentButtonSize = { -FLT_MIN, 0.0F };

        static constexpr Struct::EditorTagSearchPopupText k_addComponentPopupText = { .m_tagListHeaderText = "コンポーネントの種類",
                                                                                      .m_noItemText        = "追加できるコンポーネントがありません" };

        nlohmann::json m_editBeginTransformJson = {};
        nlohmann::json m_editBeginComponentJson = {};

        EditorTagSearchPopup m_addComponentPopup = {};
    };
}