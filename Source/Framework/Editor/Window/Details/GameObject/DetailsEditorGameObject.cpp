#include "DetailsEditorGameObject.h"

void FWK::Editor::DetailsEditorGameObject::Draw(const FWK::GameObject& a_gameObject) const
{
    // GameObject名をヘッダとして表示(編集不可)
    // 名前変更はOutlinerのF2で行うためここでは参照表示のみ
    const auto& l_gameObjectName               = a_gameObject.GetREFName();
    const auto& l_headerLabel                  = std::format            ("{} : {}", k_gameObjectNameLabel,          l_gameObjectName.empty() ? std::string{ Constant::k_gameObjectString } : l_gameObjectName);
    const auto& l_prefabUUIDLabel              = std::format            ("{} : {}", k_prefabUUIDLabel,              boost::uuids::to_string(a_gameObject.GetREFPrefabUUID()));
    const auto& l_prefabHierarchyNodeUUIDLabel = std::format            ("{} : {}", k_prefabHierarchyNodeUUIDLabel, boost::uuids::to_string(a_gameObject.GetREFPrefabHierarchyNodeUUID()));
    const auto& l_sceneInstanceUUIDLabel       = std::format            ("{} : {}", k_sceneInstanceUUIDLabel,       boost::uuids::to_string(a_gameObject.GetREFSceneInstanceUUID()));

    // 名前から描画
    ImGui::TextUnformatted(l_headerLabel.c_str());
    ImGui::TextUnformatted(l_prefabUUIDLabel.c_str());
    ImGui::TextUnformatted(l_prefabHierarchyNodeUUIDLabel.c_str());
    ImGui::TextUnformatted(l_sceneInstanceUUIDLabel.c_str());

    ImGui::Separator();

    // 選択中GameObjectのTransform + 全Componentのインスペクターを描画
    if (const auto& l_transformComponent = a_gameObject.GetVALTransformComponent().lock();
        l_transformComponent)
    {
        if (const auto& l_componentHeaderName = GameObjectTransformComponent::GetREFTypeINFO().k_name;
            ImGui::CollapsingHeader(l_componentHeaderName.data()))
        {
            l_transformComponent->EditInspector();
        }
    }
}