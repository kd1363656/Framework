#include "DetailsEditorGameObject.h"

void FWK::Editor::DetailsEditorGameObject::Draw(GameObject& a_gameObject)
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

    const auto& l_componentContainer              = a_gameObject.GetREFComponentContainer                     ();
    const auto& l_componentSmartPointerVectorList = l_componentContainer.GetREFComponentSmartPointerVectorList();
    const auto& l_componentDataList               = l_componentSmartPointerVectorList.GetREFElementDataList   ();

    // 他のコンポーネントも描画
    for (const auto& l_componentData : l_componentDataList)
    {
        const auto& l_component = l_componentData.m_type;

        if (!l_component) { continue; }

        // CollapsingHeaderが開かれなければ処理を続けない
        if (const auto& l_componentHeaderName = l_component->GetREFRuntimeTypeINFO().k_name;
            !ImGui::CollapsingHeader(l_componentHeaderName.data()))
        {
            continue;
        }

        l_component->EditInspector();
    }

    // すべてのコンポーネントのEditInspectorが終わった後に、コンポーネント追加ボタンを描画する
    DrawAddComponentButton(a_gameObject);
}

void FWK::Editor::DetailsEditorGameObject::DrawAddComponentButton(GameObject& a_gameObject)
{
    ImGui::Separator();

    // ウィンドウがボタンより狭いときは、ボタンの幅をウィンドウに合わせる
    // std::minは引数の参照を返すため、値で受け取る
    const float l_regionWidth = ImGui::GetContentRegionAvail().x;
    const auto  l_buttonWidth = std::min                    (k_addComponentButtonWidth, l_regionWidth);

    // 空いている幅からボタンの幅を引いた残りの半分だけ右へずらし、ウィンドウの幅のちょうど真ん中に置く
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (l_regionWidth - l_buttonWidth) * Constant::k_halfMagnification);

    // OpenPopupは呼び出した時点のIDスタックでポップアップのIDを作るため
    // BeginPopupを呼ぶPopupDrawer::Drawと同じスコープ(このウィンドウの中)で呼ぶ
    if (ImGui::Button(k_addComponentButtonLabel.data(), ImVec2(l_buttonWidth, k_autoFitButtonHeight)))
    {
        ImGui::OpenPopup(k_addComponentPopupLabel.data());
    }

    // ImGui::GetItemRectMin / GetItemRectMax : 直前に描画した項目(ボタン)の左上 / 右下の座標
    // ポップアップをボタンの左下から開くために使う
    const auto& l_buttonMIN = ImGui::GetItemRectMin();
    const auto& l_buttonMAX = ImGui::GetItemRectMax();

    m_addComponentPopupDrawer.Draw(k_addComponentPopupLabel,
                                   ImVec2(l_buttonMIN.x, l_buttonMAX.y),
                                   l_buttonWidth,
                                   a_gameObject);
}