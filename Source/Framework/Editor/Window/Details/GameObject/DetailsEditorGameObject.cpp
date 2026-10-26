#include "DetailsEditorGameObject.h"

void FWK::Editor::DetailsEditorGameObject::Draw(GameObject& a_gameObject, EditorManager& a_editorManager)
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

    // 選択中GameObjectの全Componentのインスペクターを描画
    // TransformComponentは一覧の先頭に固定されているので、常に一番上に表示される
    DrawComponentList(a_gameObject);

    ImGui::Separator();

    // コンポーネント一覧の一番下に、Unityと同じくAdd Componentボタンを置く
    // 一覧を描き終えてから追加するので、描いている途中の一覧が書き換わることはない
    if (const auto* l_typeINFO = m_addComponentMenu.DrawButton(k_addComponentButtonLabel, k_addComponentRootLabel, k_addComponentCategoryList);
        l_typeINFO)
    {
        AddComponent(*l_typeINFO, a_gameObject, a_editorManager);
    }
}

void FWK::Editor::DetailsEditorGameObject::DrawComponentList(const GameObject& a_gameObject) const
{
    const auto& l_componentContainer = a_gameObject.GetREFComponentContainer                   ();
    const auto& l_componentList      = l_componentContainer.GetREFComponentSmartPointerVectorList();
    const auto& l_componentDataList  = l_componentList.GetREFElementDataList                     ();

    for (const auto& l_componentData : l_componentDataList)
    {
        const auto& l_component = l_componentData.m_type;

        if (!l_component) { continue; }

        // 同じ種類のコンポーネントが2つあっても見出しの開閉が別々になるよう、コンポーネントのアドレスをIDに混ぜる
        ImGui::PushID(l_component.get());

        if (const auto& l_componentHeaderName = l_component->GetREFRuntimeTypeINFO().k_name;
            ImGui::CollapsingHeader(l_componentHeaderName.data()))
        {
            l_component->EditInspector();
        }

        ImGui::PopID();
    }
}

void FWK::Editor::DetailsEditorGameObject::AddComponent(const TypeINFORegistry::TypeINFO& a_typeINFO, GameObject& a_gameObject, EditorManager& a_editorManager) const
{
    // メニューで選ばれた型の名前は、ファクトリーに登録した名前と同じなのでそのまま生成に使える
    const auto& l_factory   = TypeAlias::GameObjectComponentSharedFactory::GetInstance();
    const auto& l_component = l_factory.Create(std::string{ a_typeINFO.k_name });

    if (!l_component)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "コンポーネントを生成できなかったため、追加に失敗しました。\nTypeName : {}", a_typeINFO.k_name.data());

        return;
    }

    // シーンを読み込む時と同じ順番で準備する
    // INIT(中身を初期状態へ) → AddComponent(持ち主とUUIDの設定) → PostDeserialize(持ち主が決まった後の準備。カメラなら射影行列を作る)
    l_component->INIT();

    auto& l_componentContainer = a_gameObject.GetMutableREFComponentContainer();

    // 1つしか持てないコンポーネントが既にある場合などはAddComponentがfalseを返す(理由はログに出る)
    if (!l_componentContainer.AddComponent(l_component)) { return; }

    l_component->PostDeserialize();

    // 追加をUndoRedoへ登録する
    auto& l_undoRedoSystem = a_editorManager.GetMutableREFUndoRedoSystem();

    l_undoRedoSystem.PushUndoCommand<AddGameObjectComponentCommand>(a_gameObject.GetREFSceneInstanceUUID(), l_component);
}