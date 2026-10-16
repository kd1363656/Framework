#include "DetailsEditorGameObject.h"

void FWK::Editor::DetailsEditorGameObject::Draw(const FWK::GameObject& a_gameObject)
{
    // GameObject名をヘッダとして表示(編集不可)
    // 名前変更はOutlinerのF2で行うためここでは参照表示のみ
    const auto& l_gameObjectName               = a_gameObject.GetREFName();
          auto  l_headerLabel                  = std::format            ("{} : {}", k_gameObjectNameLabel,          l_gameObjectName.empty() ? std::string{ Constant::k_gameObjectString } : l_gameObjectName);
          auto  l_prefabUUIDLabel              = std::format            ("{} : {}", k_prefabUUIDLabel,              boost::uuids::to_string(a_gameObject.GetREFPrefabUUID()));
          auto  l_prefabHierarchyNodeUUIDLabel = std::format            ("{} : {}", k_prefabHierarchyNodeUUIDLabel, boost::uuids::to_string(a_gameObject.GetREFPrefabHierarchyNodeUUID()));
          auto  l_sceneInstanceUUIDLabel       = std::format            ("{} : {}", k_sceneInstanceUUIDLabel,       boost::uuids::to_string(a_gameObject.GetREFSceneInstanceUUID()));

    // 名前から描画
    ImGui::TextUnformatted(l_headerLabel.c_str());
    ImGui::TextUnformatted(l_prefabUUIDLabel.c_str());
    ImGui::TextUnformatted(l_prefabHierarchyNodeUUIDLabel.c_str());
    ImGui::TextUnformatted(l_sceneInstanceUUIDLabel.c_str());

    ImGui::Separator();

    // 選択中GameObjectのTransform + 全Componentのインスペクターを描画
    DrawTransformComponent(a_gameObject);
}

void FWK::Editor::DetailsEditorGameObject::DrawTransformComponent(const FWK::GameObject& a_gameObject)
{
    const auto& l_transformComponent = a_gameObject.GetVALTransformComponent().lock();

    if (!l_transformComponent) { return; }

    if (const auto& l_componentHeaderName = GameObjectTransformComponent::GetREFTypeINFO().k_name;
        !ImGui::CollapsingHeader(l_componentHeaderName.data()))
    {
        return;
    }

    // インスペクターの全項目を1つのグループにまとめる
    // EndGroupの後のIsItemActivated / IsItemDeactivatedAfterEditは、
    // グループの中のどれか1つの項目を触り始めた / 触り終えたかを返す
    ImGui::BeginGroup();

    l_transformComponent->EditInspector();

    ImGui::EndGroup();

    // 触り始めた瞬間の値を変更前として覚えておく
    // ドラッグはクリックしたフレームではまだ値が動かないため、このフレームの値が変更前の値になる
    if (ImGui::IsItemActivated())
    {
        m_editBeginTransformJson = l_transformComponent->Serialize();
    }

    // 触り終えて値が変わっていた場合だけUndoRedo履歴へ積む
    // ドラッグ中の毎フレームの変化を1つずつ積むと、Ctrl + Zを何十回も押す必要があるため
    // 例 : 位置Xを0.0から3.0へドラッグして離す → 「0.0 → 3.0」の1つだけを積む
    if (!ImGui::IsItemDeactivatedAfterEdit()) { return; }

    const auto& l_editEndTransformJson = l_transformComponent->Serialize();

    if (l_editEndTransformJson == m_editBeginTransformJson) { return; }

    auto& l_editorManager  = EditorManager::GetInstance                 ();
    auto& l_undoRedoSystem = l_editorManager.GetMutableREFUndoRedoSystem();

    l_undoRedoSystem.PushUndoCommand<EditGameObjectTransformComponentCommand>(m_editBeginTransformJson, l_editEndTransformJson, a_gameObject.GetREFSceneInstanceUUID());
}