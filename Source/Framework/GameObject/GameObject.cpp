#include "GameObject.h"

void FWK::GameObject::INIT()
{
    m_uniqueComponentMap.clear();
    
    if (!m_transformComponent)
    {
        m_transformComponent = std::make_shared<TransformComponent>();
    }

    m_transformComponent->INIT();

    m_parent.reset();

    if (m_componentEventObserver)
    {
        m_componentEventObserver->INIT();
    }

    m_childSmartPointerVectorList.Clear    ();
    m_componentSmartPointerVectorList.Clear();

    m_componentUUIDRegistry.Clear();

    m_jsonConverter = {};

    m_prefabUUID        = {};
    m_sceneInstanceUUID = {};

    m_sceneInstanceName.clear();

    m_prefabSceneInstanceNUM = Constant::k_invalidPrefabSceneInstanceNUM;

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

void FWK::GameObject::AddComponent(const std::shared_ptr<ComponentBase>& a_component)
{
    if (!a_component)
    {
        const auto& l_gameObjectName = std::format("{}({})", m_sceneInstanceName, m_prefabSceneInstanceNUM);

        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "GameObject : {}\nコンポーネントが無効となっており割り当てに失敗しました。", l_gameObjectName);

        return;
    }

    // 派生クラスの静的IDを取得(このコンポーネントを取得時に使用)
    const auto l_staticTypeID = a_component->GetREFRuntimeTypeINFO().k_staticTypeID;
          bool l_canAdd       = false;

    // 複数持てるコンポーネントかどうかを判断して
    // 適切なstd::unordered_mapに割り当てる
    if (!a_component->IsAllowMultiple())
    {
        l_canAdd = m_uniqueComponentMap.try_emplace(l_staticTypeID, a_component).second;
    }
    else
    {
        m_multiComponentMap[l_staticTypeID].emplace_back(a_component);

        l_canAdd = true;
    }

    if (!l_canAdd)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "Component : {}\nコンポーネントの格納に失敗しました。", a_component->GetREFTypeINFO().k_name);

        return;
    }

    const std::weak_ptr<ComponentBase> l_component = a_component;

    // コンポーネントに割り当てられたUUIDを格納
    m_componentUUIDRegistry.Add          (l_component, a_component->GetMutableREFUUID());
    m_componentSmartPointerVectorList.Add(a_component);
}
void FWK::GameObject::RemoveComponent(const std::weak_ptr<ComponentBase>& a_component)
{
    const auto& l_component = a_component.lock();

    if (!l_component)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "削除対象Componentが無効のため、GameObjectから削除することができませんでした。");

        return;
    }

    const auto l_staticTypeID = l_component->GetREFRuntimeTypeINFO().k_staticTypeID;

    bool l_isRemovedFromTypeMap = false;

    if (!l_component->IsAllowMultiple())
    {
        auto l_itr = m_uniqueComponentMap.find(l_staticTypeID);

        // イテレータらから探索して、もしなければreturn
        if (l_itr == m_uniqueComponentMap.end()) { return; }

        auto l_registeredComponent = l_itr->second.lock();

        if (!l_registeredComponent)
        {
            m_uniqueComponentMap.erase(l_itr);

            return;
        }

        // 同一アドレスでない場合return
        if (l_registeredComponent != l_component) { return; }

        m_uniqueComponentMap.erase(l_itr);

        l_isRemovedFromTypeMap = true;
    }
    else
    {
        auto l_componentListITR = m_multiComponentMap.find(l_staticTypeID);

        if (l_componentListITR == m_multiComponentMap.end()) { return; }

        // コンポーネントリストをMapから取得
        auto& l_componentList = l_componentListITR->second;
        auto  l_componentITR  = l_componentList.begin();

        // リストから同じアドレスのコンポーネントを見つけ削除する
        while (l_componentITR != l_componentList.end())
        {
            auto l_registeredComponent = l_componentITR->lock();

            if (!l_registeredComponent)
            {
                l_componentITR = l_componentList.erase(l_componentITR);

                continue;
            }

            if (l_registeredComponent != l_component)
            {
                ++l_componentITR;

                continue;
            }

            l_componentList.erase(l_componentITR);

            l_isRemovedFromTypeMap = true;

            break;
        }

        if (l_componentList.empty())
        {
            m_multiComponentMap.erase(l_componentListITR);
        }
    }

    if (!l_isRemovedFromTypeMap) { return; }

    m_componentSmartPointerVectorList.RemoveSameElement(l_component);
}

bool FWK::GameObject::ApplyParent(const std::weak_ptr<GameObject>& a_child)
{
    const auto& l_child = a_child.lock();

    if (!l_child) { return false; }

    const auto& l_self = shared_from_this();

    if (!l_self) { return false; }

    // 自分自身を子にするとGameObjectの親子関係が即座に循環するため許可しない
    if (l_child == l_self) { return false; }

    if (l_child->GetREFPrefabUUID().is_nil()) { return false; }

    const auto& l_selfPrefabUUID              = l_self->GetREFPrefabUUID             ();
    const auto& l_childPrefabUUID             = l_child->GetREFPrefabUUID            ();
    const auto  l_selfPrefabSceneInstanceNUM  = l_self->GetVALPrefabSceneInstanceNUM ();
    const auto  l_childPrefabSceneInstanceNUM = l_child->GetVALPrefabSceneInstanceNUM();

    // Prefabの親子関係を構築するGameObjectは、
    // PrefabUUIDとPrefabInstanceNUMの両方を有効な値として持っている必要がある
    if (l_selfPrefabUUID.is_nil()                                                 ||
        l_childPrefabUUID.is_nil()                                                ||
        l_selfPrefabSceneInstanceNUM == Constant::k_invalidPrefabSceneInstanceNUM ||
        l_childPrefabSceneInstanceNUM == Constant::k_invalidPrefabSceneInstanceNUM)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "PrefabUUIDまたはPrefabInstanceNUMが無効なため、親子関係を構築できませんでした。");

        return false;
    }

    // 自分の祖先にしようとしていてるGameObjectを子へ設定すると
    // GameObjectそのものの親子関係が循環するため拒否する
    if (IsDescendantOf(l_child))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "GameObjectの親子関係が循環するため、親子関係を構築できませんでした。");

        return false;
    }

    const auto& l_childTransformComponent = l_child->GetVALTransformComponent().lock();

    FWK_ASSERT_RETURN_VALUE_IF(!l_childTransformComponent, "子GameObjectにTransformComponentが存在しません。", false);

    std::unordered_set<boost::uuids::uuid> l_prefabUUIDSet = {};

    // 新しい親となる自分自身からRootまで辿り
    // 同一PrefabUUIDが尾内親経路に存在しないことを確認する
    auto l_parentGameObject = l_self;

    // まずはRootからPrefabの重複があるかどうかを確認
    while (l_parentGameObject)
    {
        const auto& l_prefabUUID             = l_parentGameObject->GetREFPrefabUUID            ();
        const auto  l_prefabSceneInstanceNUM = l_parentGameObject->GetVALPrefabSceneInstanceNUM();

        // PrefabHierarchyを構成するGameObjectは、
        // PrefabUUIDとPrefabInstanceNUMの両方が有効である必要がある
        if (l_prefabUUID.is_nil() ||
            l_prefabSceneInstanceNUM == Constant::k_invalidPrefabSceneInstanceNUM)
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "親階層にPrefabInstanceではないGameObjectが存在するため、親子関係を構築できませんでした。");

            return false;
        }

        // unordered_set::emplace()の戻り値を利用して、
        // contains()とemplace()を別々に呼ばず
        // 一度の検索で重複確認と登録を行う
        if (!l_prefabUUIDSet.emplace(l_prefabUUID).second)
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "親階層に同じPrefabUUIDを持つGameObjectが存在するため、親子関係を構築できませんでした。");

            return false;
        }

        l_parentGameObject = l_parentGameObject->GetREFParent().lock();
    }

    // 親階層だけでなく、
    // 新しく接続する子GameObject以下の階層も含めて
    // 同一路線上に同じPrefabUUIDが存在しないことを確認する
    if (ContainsDuplicatePrefabUUIDRecursive(a_child, l_prefabUUIDSet))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "同じ親子経路上に同じPrefabUUIDが存在するため、親子関係を構築できませんでした。");

        return false;
    }

    const auto& l_currentParent = l_child->GetREFParent().lock();

    // すでに同じGameObjectが親の場合は二重登録しない
    if (l_currentParent == l_self) { return false; }

    // 現在の親GameObjectが存在する場合は
    // 新しい親を設定する前に親子関係を解除する
    if (l_currentParent)
    {
        l_currentParent->Unparent(l_child);

        // Unparent()に失敗して現在の親が残っている場合は
        // 新しい親GameObjectを設定しない
        if (!l_child->GetREFParent().expired())
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "以前の親GameObjectとの親子関係を解除できませんでした。");

            return false;
        }
    }

    // GameObject親子関係の構築
    l_child->SetParent(l_self);

    // 親GameObject側へ子GameObjectを登録する
    m_childSmartPointerVectorList.Add(a_child);

    // TransformComponentへ新しい親GameObjectを適用する。
    l_childTransformComponent->ApplyParent(l_self);

    return true;
}

void FWK::GameObject::Unparent(const std::weak_ptr<GameObject>& a_child)
{
    const auto& l_child = a_child.lock();

    if (!l_child) { return; }

    // 自分が親ではないGameObjectは解除しない
    if (const auto& l_currentParent = l_child->GetREFParent().lock();
        l_currentParent != shared_from_this())
    {
        return;
    }

    const auto l_childTransformComponent = l_child->GetVALTransformComponent().lock();

    FWK_ASSERT_RETURN_IF(!l_childTransformComponent, "子GameObjectにTransformComponentが存在しないため、親子関係を解除できませんでした。");

    // 子の情報を削除し、子から親の情報を削除する
    m_childSmartPointerVectorList.RemoveSameElement(a_child);
    l_child->GetMutableREFParent                   ().reset();

    // 親がいなくても行列を計算できるようにする
    l_childTransformComponent->ApplyStandalone();
}

bool FWK::GameObject::ContainsDuplicatePrefabUUIDRecursive(const std::weak_ptr<GameObject>& a_gameObject, std::unordered_set<boost::uuids::uuid>& a_prefabUUIDSet) const
{
    const auto& l_gameObject = a_gameObject.lock();

    if (!l_gameObject ||
        l_gameObject->GetVALIsDestroyed())
    {
        return false;
    }

    const auto& l_prefabUUID        = l_gameObject->GetREFPrefabUUID            ();
    const auto  l_prefabInstanceNUM = l_gameObject->GetVALPrefabSceneInstanceNUM();

    // Prefab情報を持っていないGameObjectは、
    // PrefabUUIDによる循環確認の対象には含めない
    if (l_prefabUUID.is_nil() ||
        l_prefabInstanceNUM == Constant::k_invalidPrefabSceneInstanceNUM)
    {
        return false;
    }

    // 現在確認している親子経路に
    // 同じPrefabUUIDが既に存在する場合は循環している
    if (!a_prefabUUIDSet.emplace(l_prefabUUID).second) { return true; }

    const auto& l_childSmartPointerVectorList = l_gameObject->GetREFChildSmartPointerVectorList     ();
    const auto& l_childDataList               = l_childSmartPointerVectorList.GetREFElementDataList();

    for (const auto& l_childData : l_childDataList)
    {
        if (const auto& l_child = l_childData.m_type.lock();
            !l_child ||
            l_child->GetVALIsDestroyed())
        {
            continue;
        }

        if (ContainsDuplicatePrefabUUIDRecursive(l_childData.m_type, a_prefabUUIDSet))
        {
            // 呼びだし元へ戻る前に
            // 小野GameObjectで追加したUUIDを取り除く
            a_prefabUUIDSet.erase(l_prefabUUID);

            return true;
        }
    }

    // 兄弟は同じ親子経路ではないため
    // 現在のGameObjectの以下の確認が終了した時点で解除する
    a_prefabUUIDSet.erase(l_prefabUUID);

    return false;
}

bool FWK::GameObject::IsDescendantOf(const std::shared_ptr<GameObject>& a_ancestor) const
{
    if (!a_ancestor) { return false; }

    auto l_parent = m_parent.lock();

    // 自身から親方向へさかのぼって
    // してGameObjectが祖先に存在するか確認する
    while (l_parent)
    {
        if (l_parent == a_ancestor) { return true; }

        l_parent = l_parent->GetREFParent().lock();
    }

    return false;
}