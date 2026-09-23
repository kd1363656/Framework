#include "GameObject.h"

void FWK::GameObject::INIT()
{
    if (!m_transformComponent)
    {
        m_transformComponent = std::make_shared<TransformComponent>();
    }

    m_transformComponent->INIT();

    if (m_componentEventObserver)
    {
        m_componentEventObserver->INIT();
    }

    m_jsonConverter = {};

    m_prefabUUID        = {};
    m_sceneInstanceUUID = {};

    m_name.clear();

    m_isDestroyed = false;
}

void FWK::GameObject::Deserialize(const nlohmann::json& a_rootJson, std::unordered_set<boost::uuids::uuid>& a_prefabUUIDSet, Scene& a_scene)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(weak_from_this(),
                                a_rootJson,
                                a_prefabUUIDSet,
                                a_scene);
}
bool FWK::GameObject::DeserializePrefab(const nlohmann::json&                                                  a_rootJson,
                                              std::vector<Struct::ChildDeserializeData>&                       a_childDeserializeDataList,
                                              Utility::SmartPointerVectorList<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorList,
                                              std::unordered_set<boost::uuids::uuid>&                          a_parentPrefabUUIDSet,
                                              Scene&                                                           a_scene)
{
    if (a_rootJson.is_null()) { return false; }

    return m_jsonConverter.DeserializePrefab(weak_from_this(),
                                             a_rootJson,
                                             a_childDeserializeDataList,
                                             a_parentPrefabUUIDSet,
                                             a_componentSmartPointerVectorList,
                                             a_scene);
}

bool FWK::GameObject::DeserializePrefabInstance(const nlohmann::json& a_prefabJson, std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList, Scene& a_scene)
{
    if (a_prefabJson.is_null()) { return false; }

    return m_jsonConverter.DeserializePrefabInstance(weak_from_this(),
                                                     a_prefabJson,
                                                     a_childDeserializeDataList,
                                                     a_scene);
}

bool FWK::GameObject::DeserializeScene(const nlohmann::json&                                                  a_rootJson,
                                             std::vector<Struct::ChildDeserializeData>&                       a_childDeserializeDataList,
                                             Utility::SmartPointerVectorList<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorList,
                                             Scene&                                                           a_scene)
{
    if (a_rootJson.is_null()) { return false; }

    return m_jsonConverter.DeserializeScene(a_rootJson,
                                            a_childDeserializeDataList,
                                            a_componentSmartPointerVectorList,
                                            *this,
                                            a_scene);
}

void FWK::GameObject::RecursiveAddComponent(const Utility::SmartPointerVectorList<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorList, std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList)
{
    // 親のコンポーネントを親のコンポーネントリストに追加
    const auto& l_componentDataList = a_componentSmartPointerVectorList.GetREFElementDataList();

    for (const auto& l_componentData : l_componentDataList)
    {
        const auto& l_component = l_componentData.m_type;

        if (!l_component) { continue; }

        AddComponent(l_component);
    }

    // 子のコンポーネントを子のコンポーネントリストに追加
    for (auto& l_childLoad : a_childDeserializeDataList)
    {
        const auto& l_child = l_childLoad.m_self;

        if (!l_child) { continue; }

        // 子のコンポーネントも再帰的に追加していく
        l_child->RecursiveAddComponent(l_childLoad.m_componentSmartPointerVectorList, l_childLoad.m_childDeserializeDataList);
    }
}
bool FWK::GameObject::RecursiveAddChild(std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList, Scene& a_scene)
{
    bool l_isAllChildAdded = true;

    // 親子関係を再帰的に構築
    for (auto& l_childLoad : a_childDeserializeDataList)
    {
        const auto& l_child = l_childLoad.m_self;

        if (!l_child) { continue; }

        // 同じPrefab名が親経路に存在する場合や、
        // GameObjectの親子関係を構築できなかった場合は追加しない
        if (!ApplyParent(l_child))
        {
            l_isAllChildAdded = false;

            continue;
        }

        // 親子関係を構築できたGameObjectだけをSceneへ追加する
        a_scene.AddGameObject(l_child);

        // 子のPrefabUUIDがSetへ入った状態で
        // 孫以下の親子関係を構築する
        if (!l_child->RecursiveAddChild(l_childLoad.m_childDeserializeDataList, a_scene))
        {
            l_isAllChildAdded = false;
        }
    }

    return l_isAllChildAdded;
}

void FWK::GameObject::PostDeserialize()
{
    FWK_ASSERT_RETURN_IF(!m_transformComponent, "TransformComponentが存在しません、TransformComponentは必ず存在するべきComponentです。");

    m_transformComponent->SetOwner       (weak_from_this());
    m_transformComponent->PostDeserialize();

    const auto& l_componentDataList = m_componentSmartPointerVectorList.GetREFElementDataList();

    for (const auto& l_componentData : l_componentDataList)
    {
        const auto& l_component = l_componentData.m_type;

        if (!l_component) { continue; }

        l_component->SetOwner       (weak_from_this());
        l_component->PostDeserialize();
    }
}

void FWK::GameObject::EarlyUpdate() const
{
    const auto& l_componentDataList = m_componentSmartPointerVectorList.GetREFElementDataList();

    for (const auto& l_componentData : l_componentDataList)
    {
        const auto& l_component = l_componentData.m_type;

        if (!l_component) { continue; }

        l_component->EarlyUpdate();
    }
}
void FWK::GameObject::Update() const
{
    const auto& l_componentDataList = m_componentSmartPointerVectorList.GetREFElementDataList();

    for (const auto& l_componentData : l_componentDataList)
    {
        const auto& l_component = l_componentData.m_type;

        if (!l_component) { continue; }

        l_component->Update();
    }
}
void FWK::GameObject::LateUpdate() const
{
    const auto& l_componentDataList = m_componentSmartPointerVectorList.GetREFElementDataList();

    for (const auto& l_componentData : l_componentDataList)
    {
        const auto& l_component = l_componentData.m_type;

        if (!l_component) { continue; }

        l_component->LateUpdate();
    }
}
void FWK::GameObject::PostLateUpdate() const
{
    FWK_ASSERT_RETURN_IF(!m_transformComponent, "TransformComponentが存在しません、TransformComponentは必ず存在するべきComponentです。");

    // 当たり判定などで微調整された行列を確定
    m_transformComponent->PostLateUpdate();

    // 行列確定後に決まる処理を更新する
    const auto& l_componentDataList = m_componentSmartPointerVectorList.GetREFElementDataList();

    for (const auto& l_componentData : l_componentDataList)
    {
        const auto& l_component = l_componentData.m_type;

        if (!l_component) { continue; }

        l_component->PostLateUpdate();
    }
}

void FWK::GameObject::Destroy()
{
    m_isDestroyed = true;

    // 子も削除フラグを立てる
    // もし親が削除されて連動して消されたくないような局面が出てきたら
    // その時に処理を書き換えるようにすること、基本は親と連動して削除フラグを立てる
    const auto& l_childDataList = m_childSmartPointerVectorList.GetMutableREFElementDataList();

    for (const auto& l_childData : l_childDataList)
    {
        auto l_child = l_childData.m_type.lock();

        if (!l_child) { continue; }

        l_child->Destroy();
    }
}

void FWK::GameObject::EditInspector()
{
    FWK_ASSERT_RETURN_IF(!m_transformComponent, "TransformComponentが存在しません、TransformComponentは必ず存在するべきComponentです。");

    m_transformComponent->EditInspector();

    const auto& l_componentDataList = m_componentSmartPointerVectorList.GetREFElementDataList();

    for (const auto& l_componentData : l_componentDataList)
    {
        const auto& l_component = l_componentData.m_type;

        if (!l_component) { continue; }

        l_component->EditInspector();
    }
}

nlohmann::json FWK::GameObject::SerializeScene() const
{
    return m_jsonConverter.SerializeScene(*this);
}
nlohmann::json FWK::GameObject::SerializePrefab() const
{
    return m_jsonConverter.SerializePrefab(*this);
}