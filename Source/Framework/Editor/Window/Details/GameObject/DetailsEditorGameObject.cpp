#include "DetailsEditorGameObject.h"

void FWK::Editor::DetailsEditorGameObject::Draw(const FWK::GameObject& a_gameObject)
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
    DrawTransformComponent(a_gameObject);
    DrawComponentList     (a_gameObject);

    // 一番下にコンポーネントの追加ボタン
    DrawAddComponentButton(a_gameObject);
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

    l_undoRedoSystem.PushUndoCommand<EditGameObjectTransformComponentSceneCommand>(m_editBeginTransformJson, l_editEndTransformJson, a_gameObject.GetREFSceneInstanceUUID());
}
void FWK::Editor::DetailsEditorGameObject::DrawComponentList(const FWK::GameObject& a_gameObject)
{
    const auto& l_componentContainer = a_gameObject.GetREFComponentContainer                     ();
    const auto& l_componentDataList  = l_componentContainer.GetREFComponentSmartPointerVectorList();
    const auto& l_elementDataList    = l_componentDataList.GetREFElementDataList                 ();

    // 削除は一覧を回し終えてから行う
    // 回している途中で一覧から外すと、次の要素の位置がずれて飛ばしたり壊れた要素を読んだりするため
    std::weak_ptr<GameObjectComponentBase> l_removeComponent = {};

    for (const auto& l_elementData : l_elementDataList)
    {
        const auto& l_component = l_elementData.m_type;

        if (!l_component) { continue; }

        DrawComponent(l_component, a_gameObject, l_removeComponent);
    }

    if (l_removeComponent.expired()) { return; }

    auto& l_editorManager  = EditorManager::GetInstance                 ();
    auto& l_undoRedoSystem = l_editorManager.GetMutableREFUndoRedoSystem();

    // 削除はUndoで戻せるように、記録してから外す(最初のRedoで外す)
    l_undoRedoSystem.ExecuteUndoCommand<RemoveGameObjectComponentSceneCommand>(l_removeComponent, a_gameObject.GetREFSceneInstanceUUID());
}
void FWK::Editor::DetailsEditorGameObject::DrawAddComponentButton(const FWK::GameObject& a_gameObject)
{
    ImGui::Separator();

    // 横幅いっぱいのボタン(幅に-FLT_MINを渡すと、残りの幅をすべて使う)
    if (ImGui::Button(k_addComponentButtonLabel.data(), k_addComponentButtonSize))
    {
        ImGui::OpenPopup(k_addComponentPopupLabel.data());
    }

    // ポップアップはボタンの真下に、ボタンと同じ幅で開く
    // GetItemRectMin / GetItemRectMaxは直前に描画した項目(ボタン)の左上と右下
    const auto& l_buttonMIN = ImGui::GetItemRectMin();
    const auto& l_buttonMAX = ImGui::GetItemRectMax();

    // 開いていないフレームは、一覧を作らない
    if (!ImGui::IsPopupOpen(k_addComponentPopupLabel.data())) { return; }

    // タグ(カテゴリー)と、その中の追加できるコンポーネントの一覧を渡し、決定したコンポーネントの型の名前を受け取る
    // 決定しなかったフレームは空が返る
    const auto& l_addableTagGroupList = FetchVALAddComponentTagGroupList(a_gameObject);
    const auto& l_popupPosition       = ImVec2{ l_buttonMIN.x, l_buttonMAX.y };
    const float l_popupWidth          = l_buttonMAX.x - l_buttonMIN.x;

    if (const auto& l_decidedTypeName = m_addComponentPopup.Draw(l_addableTagGroupList,
                                                                 k_addComponentPopupLabel,
                                                                 l_popupPosition,
                                                                 k_addComponentPopupText,
                                                                 l_popupWidth);
        !l_decidedTypeName.empty())
    {
        AddComponent(l_decidedTypeName, a_gameObject);
    }
}
void FWK::Editor::DetailsEditorGameObject::DrawComponent(const std::shared_ptr<GameObjectComponentBase>& a_component,
                                                               const FWK::GameObject&                          a_gameObject,
                                                         std::weak_ptr<GameObjectComponentBase>&   a_removeComponent)
{
    // 同じ種類のコンポーネントが2つあってもImGuiのIDがぶつからないように、実体のアドレスでIDを分ける
    ImGui::PushID(a_component.get());

    const auto& l_typeINFO = a_component->GetREFRuntimeTypeINFO();
    const bool  l_isOpen   = ImGui::CollapsingHeader           (l_typeINFO.k_name.data(), ImGuiTreeNodeFlags_DefaultOpen);

    // 見出しを右クリックしたら削除のメニューを出す
    if (ImGui::BeginPopupContextItem(k_componentContextMenuLabel.data()))
    {
        if (ImGui::MenuItem(k_removeComponentLabel.data()))
        {
            a_removeComponent = a_component;
        }

        ImGui::EndPopup();
    }

    if (!l_isOpen)
    {
        ImGui::PopID();

        return;
    }

    // 有効のチェックとインスペクターの全項目を1つのグループにまとめ、
    // Transformと同じく「触り始め → 触り終え」を1回の編集としてUndoへ積む
    ImGui::BeginGroup();

    if (bool l_isEnable = !a_component->GetVALIsDisable();
        ImGui::Checkbox(k_componentEnableLabel.data(), &l_isEnable))
    {
        a_component->SetIsDisable(!l_isEnable);

        // 無効なコンポーネントはシーン読み込み時に準備(PostDeserialize)されていないので、有効にしたときに準備する
        if (l_isEnable)
        {
            a_component->PostDeserialize();
        }
    }

    a_component->EditInspector();

    ImGui::EndGroup();

    if (ImGui::IsItemActivated())
    {
        m_editBeginComponentJson = a_component->Serialize();
    }

    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        if (const auto& l_editEndComponentJson = a_component->Serialize();
            l_editEndComponentJson != m_editBeginComponentJson)
        {
            auto& l_editorManager  = EditorManager::GetInstance                 ();
            auto& l_undoRedoSystem = l_editorManager.GetMutableREFUndoRedoSystem();

            l_undoRedoSystem.PushUndoCommand<EditGameObjectComponentSceneCommand>(m_editBeginComponentJson,
                                                                                  l_editEndComponentJson,
                                                                                  a_gameObject.GetREFSceneInstanceUUID(),
                                                                                  a_component->GetREFUUID());
        }
    }

    ImGui::PopID();
}

void FWK::Editor::DetailsEditorGameObject::AddComponent(const std::string_view& a_typeName, const FWK::GameObject& a_gameObject) const
{
    // 型の名前からコンポーネントを作る(シーンのJsonを読み込むときと同じ仕組み)
    // GenericFactoryはstd::stringで受け取るため、押されたときだけstd::stringを作る
    const auto& l_factory   = TypeAlias::GameObjectComponentSharedFactory::GetInstance();
    const auto& l_component = l_factory.Create                                        (std::string{ a_typeName });

    if (!l_component)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "コンポーネントの作成に失敗しました。\nType : {}", a_typeName);

        return;
    }

    auto& l_editorManager  = EditorManager::GetInstance                 ();
    auto& l_undoRedoSystem = l_editorManager.GetMutableREFUndoRedoSystem();

    // 1つしか持てない種類かどうかの確認は、一覧を作るとき(CanAddComponent)に済んでいる
    // 追加できないコンポーネントはポップアップに並ばないため、ここでは調べずにそのまま追加する
    // 追加はUndoで外せるように、記録してから追加する(最初のRedoで追加する)
    l_undoRedoSystem.ExecuteUndoCommand<AddGameObjectComponentSceneCommand>(l_component, a_gameObject.GetREFSceneInstanceUUID());
}

bool FWK::Editor::DetailsEditorGameObject::CanAddComponent(const std::string_view& a_typeName, const FWK::GameObject& a_gameObject) const
{
    const auto& l_componentContainer = a_gameObject.GetREFComponentContainer                     ();
    const auto& l_componentDataList  = l_componentContainer.GetREFComponentSmartPointerVectorList();

    // 1つしか持てないコンポーネントを、すでに追加している場合は追加できない
    // 複数持てるコンポーネント(IsAllowMultipleがtrue)は、何個あっても追加できる
    // 例 : カメラを持っているGameObjectでは、追加の一覧からGameObjectCameraComponentが消える
    return std::ranges::none_of(l_componentDataList.GetREFElementDataList(),
                                [&a_typeName](const auto& a_elementData)
                                {
                                    const auto& l_component = a_elementData.m_type;

                                    if (!l_component)                   { return false; }
                                    if (l_component->IsAllowMultiple()) { return false; }

                                    return l_component->GetREFRuntimeTypeINFO().k_name == a_typeName;
                                });
}

std::vector<FWK::Struct::TaggedFactoryTagGroup> FWK::Editor::DetailsEditorGameObject::FetchVALAddComponentTagGroupList(const FWK::GameObject& a_gameObject) const
{
    // タグ(カテゴリー)ごとに、そのタグのコンポーネントのうち追加できるものだけを集める
    // タグとコンポーネントの対応は、各コンポーネントのヘッダーの一番下の
    // FWK_REGISTER_TAGGED_FACTORYで登録されている
    // 並び順はFramework.hでコンポーネントのヘッダーを読み込んだ順(登録した順)
    // ファクトリーの一覧と同じ形(Struct::TaggedFactoryTagGroup)で返すため、ポップアップへそのまま渡せる
    // 例 : カメラを持っていないGameObject → { カメラ : { GameObjectCameraComponent } }
    //      カメラを持っているGameObject   → { カメラ : {} }(中が空のタグはポップアップに並ばない)
          std::vector<Struct::TaggedFactoryTagGroup> l_addableTagGroupList = {};
    const auto&                                      l_taggedFactory       = TaggedFactory<GameObjectComponentBase>::GetInstance();
    const auto&                                      l_tagGroupList        = l_taggedFactory.GetREFTagGroupList                  ();

    l_addableTagGroupList.reserve(l_tagGroupList.size());

    for (const auto& l_tagGroup : l_tagGroupList)
    {
        auto& l_addableTagGroup = l_addableTagGroupList.emplace_back();

        l_addableTagGroup.m_tag = l_tagGroup.m_tag;

        for (const auto& l_typeName : l_tagGroup.m_typeNameList)
        {
            if (!CanAddComponent(l_typeName, a_gameObject)) { continue; }

            l_addableTagGroup.m_typeNameList.emplace_back(l_typeName);
        }
    }

    return l_addableTagGroupList;
}