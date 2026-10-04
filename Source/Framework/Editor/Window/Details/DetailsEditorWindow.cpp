#include "DetailsEditorWindow.h"

void FWK::Editor::DetailsEditorWindow::Draw(EditorManager& a_editorManager)
{
    if (!ImGui::Begin(k_editorName.data()))
    {
        ImGui::End();

        return;
    }

    // このウィンドウがクリックされているかどうかを判定
    ReportActiveWindowIfMouseClicked(a_editorManager);

    Utility::IMGUIDelayedTooltip(k_thisWindowExplanationLabel);

    // EditorManagerが保持するGameObject選択状態を取得
    // ヒエラルキー(Outliner)とビューポートで選択状態を共有している
    auto& l_gameObjectSelectionState = a_editorManager.GetMutableREFGameObjectSelectionState();

    // 破棄済み、参照切れのGameObjectを選択状態から取り除く
    l_gameObjectSelectionState.SweepUnavailableGameObjects();

    // 最後に選択したGameObjectを取得
    // 複数選択時は最後に選択したGameObjectのみインスペクター描画の対象にする
    const auto& l_selectedGameObject = l_gameObjectSelectionState.FindVALLastSelectedGameObject().lock();
 
    // 未選択、または無効なら何も描画しない
    if (!l_selectedGameObject ||
        l_selectedGameObject->GetVALIsDestroyed())
    {
        // 選択しているゲームオブジェクトがないことを描画
        ImGui::TextDisabled(k_noSelectionLabel.data());

        ImGui::End();

        return;
    }

    DrawGameObjectDetails(*l_selectedGameObject);

    ImGui::End();
}

void FWK::Editor::DetailsEditorWindow::DrawGameObjectDetails(const FWK::GameObject& a_gameObject) const
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