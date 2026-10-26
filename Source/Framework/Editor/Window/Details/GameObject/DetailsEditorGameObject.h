#pragma once

namespace FWK::Editor
{
    class DetailsEditorGameObject final
    {
    public:

         DetailsEditorGameObject() = default;
        ~DetailsEditorGameObject() = default;

        void Draw(GameObject& a_gameObject, EditorManager& a_editorManager);

    private:

        void DrawComponentList(const GameObject& a_gameObject) const;

        void AddComponent(const TypeINFORegistry::TypeINFO& a_typeINFO, GameObject& a_gameObject, EditorManager& a_editorManager) const;

        EditorAddMenu m_addComponentMenu = {};

        static constexpr std::string_view k_gameObjectHeaderLabel        = "ゲームオブジェクト基本情報    ";
        static constexpr std::string_view k_gameObjectNameLabel          = "名前                          ";
        static constexpr std::string_view k_prefabUUIDLabel              = "プレハブUUID                  ";
        static constexpr std::string_view k_prefabHierarchyNodeUUIDLabel = "プレハブヒエラルキーノードUUID";
        static constexpr std::string_view k_sceneInstanceUUIDLabel       = "シーンインスタンスUUID        ";

        static constexpr std::string_view k_addComponentButtonLabel = "Add Component";
        static constexpr std::string_view k_addComponentRootLabel   = "Component";

        // Add Componentのカテゴリー選択画面に、この順番で並ぶ
        // コンポーネントに新しいタグを付けたら、ここにも追加しないとメニューに出てこない
        inline static const std::vector<std::string_view> k_addComponentCategoryList =
        {
            Constant::k_typeINFOCameraTagName
        };
    };
}