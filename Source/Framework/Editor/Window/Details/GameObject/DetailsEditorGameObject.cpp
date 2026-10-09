#include "DetailsEditorGameObject.h"

void FWK::Editor::DetailsEditorGameObject::Draw(const std::weak_ptr<GameObject>& a_gameObject)
{
    const auto& l_gameObject = a_gameObject.lock();

    if (!l_gameObject) { return; }

    // GameObject名をヘッダとして表示(編集不可)
    // 名前変更はOutlinerのF2で行うためここでは参照表示のみ
    const auto& l_gameObjectName               = l_gameObject->GetREFName();
    const auto& l_headerLabel                  = std::format             ("{} : {}", k_gameObjectNameLabel,          l_gameObjectName.empty() ? std::string{ Constant::k_gameObjectString } : l_gameObjectName);
    const auto& l_prefabUUIDLabel              = std::format             ("{} : {}", k_prefabUUIDLabel,              boost::uuids::to_string(l_gameObject->GetREFPrefabUUID()));
    const auto& l_prefabHierarchyNodeUUIDLabel = std::format             ("{} : {}", k_prefabHierarchyNodeUUIDLabel, boost::uuids::to_string(l_gameObject->GetREFPrefabHierarchyNodeUUID()));
    const auto& l_sceneInstanceUUIDLabel       = std::format             ("{} : {}", k_sceneInstanceUUIDLabel,       boost::uuids::to_string(l_gameObject->GetREFSceneInstanceUUID()));

    // 名前から描画
    ImGui::TextUnformatted(l_headerLabel.c_str());
    ImGui::TextUnformatted(l_prefabUUIDLabel.c_str());
    ImGui::TextUnformatted(l_prefabHierarchyNodeUUIDLabel.c_str());
    ImGui::TextUnformatted(l_sceneInstanceUUIDLabel.c_str());

    ImGui::Separator();

    // 選択中GameObjectのTransform + 全Componentのインスペクターを描画
    if (const auto& l_transformComponent = l_gameObject->GetVALTransformComponent().lock();
        l_transformComponent)
    {
        if (const auto& l_componentHeaderName = GameObjectTransformComponent::GetREFTypeINFO().k_name;
            ImGui::CollapsingHeader(l_componentHeaderName.data()))
        {
            l_transformComponent->EditInspector();
        }
    }

    const auto& l_componentContainer              = l_gameObject->GetREFComponentContainer                    ();
    const auto& l_componentSmartPointerVectorList = l_componentContainer.GetREFComponentSmartPointerVectorList();
    const auto& l_componentDataList               = l_componentSmartPointerVectorList.GetREFElementDataList   ();

    // メニューで「削除」が選ばれたコンポーネント
    // 描画のループの途中でコンテナのリストを変更すると、回している最中のリストが壊れるため、
    // ここへ覚えておき、ループが終わってから削除する
    std::shared_ptr<GameObjectComponentBase> l_removeTarget = nullptr;

    // 他のコンポーネントも描画
    for (const auto& l_componentData : l_componentDataList)
    {
        const auto& l_component = l_componentData.m_type;

        if (!l_component) { continue; }

        // 同じ型のコンポーネントを複数持つ場合、ヘッダーの文字が同じでImGuiのIDが重なるため、
        // コンポーネントのUUIDでIDを分ける
        ImGui::PushID(boost::uuids::to_string(l_component->GetREFUUID()).c_str());

        // 同じ行の右端に置くメニューボタンが、ヘッダーの上でもクリックを受け取れるようにする
        ImGui::SetNextItemAllowOverlap();

        const auto& l_componentHeaderName = l_component->GetREFRuntimeTypeINFO().k_name;
        const bool  l_isHeaderOpen        = ImGui::CollapsingHeader           (l_componentHeaderName.data());

        // ヘッダーの右端に縦三点のメニューボタンを置く。「削除」が選ばれたら覚えておく
        if (DrawComponentMenuButton())
        {
            l_removeTarget = l_component;
        }

        // CollapsingHeaderが開かれているときだけ、インスペクターを描画する
        if (l_isHeaderOpen)
        {
            l_component->EditInspector();
        }

        ImGui::PopID();
    }

    // ループが終わったので、削除が選ばれていれば削除する
    if (l_removeTarget)
    {
        RemoveComponent(a_gameObject, l_removeTarget);
    }

    // すべてのコンポーネントのEditInspectorが終わった後に、コンポーネント追加ボタンを描画する
    DrawAddComponentButton(a_gameObject);
}

void FWK::Editor::DetailsEditorGameObject::DrawAddComponentButton(const std::weak_ptr<GameObject>& a_gameObject)
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
    if (ImGui::Button(k_addComponentButtonLabel.data(), ImVec2{ l_buttonWidth, k_autoFitButtonHeight }))
    {
        ImGui::OpenPopup(k_addComponentPopupLabel.data());
    }

    // ImGui::GetItemRectMin / GetItemRectMax : 直前に描画した項目(ボタン)の左上 / 右下の座標
    // ポップアップをボタンの左下から開くために使う
    const auto& l_buttonMIN = ImGui::GetItemRectMin();
    const auto& l_buttonMAX = ImGui::GetItemRectMax();

    m_addComponentPopupDrawer.Draw(a_gameObject,
                                   k_addComponentPopupLabel,
                                   ImVec2{ l_buttonMIN.x, l_buttonMAX.y },
                                   l_buttonWidth);
}

bool FWK::Editor::DetailsEditorGameObject::DrawComponentMenuButton() const
{
    const auto& l_style = ImGui::GetStyle();

    // ヘッダーの右端にボタンを置く
    // CollapsingHeaderの直後はカーソルが次の行にあるが、SameLineで位置を指定すると、ヘッダーと同じ行の右端へ戻れる
    // ボタンのX座標 = カーソルのX + 残りの幅 - ボタンの幅
    // ボタンの幅 = アイコンの幅 + 左右の余白(FramePadding)
    // CalcTextSizeの第3引数をtrueにすると、ラベルの"##"より後ろ(ImGuiのID)を幅に含めない
    const float l_buttonWidth     = ImGui::CalcTextSize (k_componentMenuButtonLabel.data(), nullptr, true).x + l_style.FramePadding.x * k_framePaddingBothSidesNUM;
    const float l_buttonPositionX = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - l_buttonWidth;

    ImGui::SameLine(l_buttonPositionX);

    // ボタンの背景は透明にして、ヘッダーの上にアイコンだけが見えるようにする
    ImGui::PushStyleColor(ImGuiCol_Button, Constant::k_imguiTransparentColor);

    const bool l_isClicked = ImGui::SmallButton(k_componentMenuButtonLabel.data());

    ImGui::PopStyleColor();

    // OpenPopupは呼び出した時点のIDスタックでポップアップのIDを作るため、BeginPopupと同じスコープ(同じコンポーネントのID)で呼ぶ
    if (l_isClicked)
    {
        ImGui::OpenPopup(k_componentMenuPopupLabel.data());
    }

    bool l_isRemoveSelected = false;

    if (ImGui::BeginPopup(k_componentMenuPopupLabel.data()))
    {
        // MenuItemは、選ばれたフレームだけtrueを返し、選ぶとポップアップも閉じる
        l_isRemoveSelected = ImGui::MenuItem(k_removeMenuLabel.data());

        ImGui::EndPopup();
    }

    return l_isRemoveSelected;
}

void FWK::Editor::DetailsEditorGameObject::RemoveComponent(const std::weak_ptr<GameObject>& a_gameObject, const std::shared_ptr<GameObjectComponentBase>& a_component) const
{
    const auto& l_gameObject = a_gameObject.lock();

    if (!l_gameObject) { return; }

    auto& l_componentContainer = l_gameObject->GetMutableREFComponentContainer();

    // Undoで元の位置へ戻せるよう、削除する前のコンポーネントの並び(UUID)を記録しておく
    const auto& l_beforeComponentUUIDList = l_componentContainer.FetchVALComponentUUIDList();

    l_componentContainer.RemoveComponent(a_component);

    // 削除を終えてから、履歴へ積む
    // コマンドがshared_ptrを持つため、コンテナから外れたコンポーネントもUndoまで消えない
    auto& l_editorManager  = EditorManager::GetInstance                 ();
    auto& l_undoRedoSystem = l_editorManager.GetMutableREFUndoRedoSystem();

    l_undoRedoSystem.PushUndoCommand<RemoveGameObjectComponentCommand>(l_beforeComponentUUIDList, a_component, l_gameObject->GetREFSceneInstanceUUID());
}