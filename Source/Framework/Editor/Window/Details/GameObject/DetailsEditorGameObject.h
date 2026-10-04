#pragma once

namespace FWK::Editor
{
    class DetailsEditorGameObject final
    {
    public:

         DetailsEditorGameObject() = default;
        ~DetailsEditorGameObject() = default;

        void Draw(const FWK::GameObject& a_gameObject) const;

    private:

        static constexpr std::string_view k_gameObjectHeaderLabel        = "ゲームオブジェクト基本情報    ";
        static constexpr std::string_view k_gameObjectNameLabel          = "名前                          ";
        static constexpr std::string_view k_prefabUUIDLabel              = "プレハブUUID                  ";
        static constexpr std::string_view k_prefabHierarchyNodeUUIDLabel = "プレハブヒエラルキーノードUUID";
        static constexpr std::string_view k_sceneInstanceUUIDLabel       = "シーンインスタンスUUID        ";
    };
}
