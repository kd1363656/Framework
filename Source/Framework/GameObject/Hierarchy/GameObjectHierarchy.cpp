#include "GameObjectHierarchy.h"

void FWK::GameObjectHierarchy::INIT()
{
    m_prefabRemovedChildUUIDSet.clear();

    m_parent = {};
    m_owner  = {};

    m_childSmartPointerVectorList.Clear();

    m_childUUIDRegistry.Clear();

    m_jsonConverter = {};
}

void FWK::GameObjectHierarchy::DeserializeScene(const nlohmann::json&              a_rootJson, 
                                                const nlohmann::json&              a_prefabJson,
                                                const SceneGameObjectPrefabSystem& a_prefabSystem, 
                                                      Scene&                       a_scene)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.DeserializeScene(a_rootJson, 
                                     a_prefabJson, 
                                     a_prefabSystem,
                                     *this, 
                                     a_scene);
}
void FWK::GameObjectHierarchy::DeserializePrefab(const nlohmann::json& a_rootJson, const SceneGameObjectPrefabSystem& a_prefabSystem, Scene& a_scene)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.DeserializePrefab(a_rootJson, 
                                      a_prefabSystem,
                                      *this, 
                                      a_scene);
}

void FWK::GameObjectHierarchy::PostDeserialize()
{
    const auto& l_gameObject = m_owner.lock();

    if (!l_gameObject) { return; }

    l_gameObject->PostDeserialize();

    const auto& l_childDataList = m_childSmartPointerVectorList.GetMutableREFElementDataList();

    for (const auto& l_childData : l_childDataList)
    {
        const auto& l_child = l_childData.m_type.lock();

        if (!l_child) { continue; }

        l_child->PostDeserialize();
    }
}

nlohmann::json FWK::GameObjectHierarchy::Serialize(SceneGameObjectPrefabSystem& a_prefabSystem) const
{
    return m_jsonConverter.Serialize(*this, a_prefabSystem);
}
nlohmann::json FWK::GameObjectHierarchy::SerializeDiff(const nlohmann::json& a_prefabJson, SceneGameObjectPrefabSystem& a_prefabSystem) const
{
    return m_jsonConverter.SerializeDiff(a_prefabJson, *this, a_prefabSystem);
}

void FWK::GameObjectHierarchy::Clone(GameObjectHierarchy& a_cloneHierarchy, Scene& a_scene) const
{
    // Prefab由来の子の削除追跡を複写する
    // Prefabインスタンスのクローンでは削除意図も差分の一部のため必要
    // 内部子のクローンでDetachされる場合はDetachFromPrefabがクリアするため無害
    for (const auto& l_removedUUID : m_prefabRemovedChildUUIDSet)
    {
        a_cloneHierarchy.AddPrefabRemovedUUID(l_removedUUID);
    }
 
    // クローン側HierarchyのOwnerがクローンされた子たちの親になる
    const auto& l_cloneParent = a_cloneHierarchy.GetREFOwner();
 
    FWK_ASSERT_RETURN_IF(l_cloneParent.expired(), "クローン側HierarchyのOwnerが無効なため、子のクローンに失敗しました。");
 
    const auto& l_childDataList = m_childSmartPointerVectorList.GetREFElementDataList();
 
    for (const auto& l_childData : l_childDataList)
    {
        const auto& l_child = l_childData.m_type.lock();
 
        // 無効または破棄済みの子はクローン対象から外す
        if (!l_child ||
            l_child->GetVALIsDestroyed())
        {
            continue;
        }
 
        const auto& l_childPrefabHierarchyNodeUUID = l_child->GetREFPrefabHierarchyNodeUUID();

        // 子自身のCloneが生成・接続・Scene登録・子孫の再帰までを行う
        // コピー元のPrefabHierarchyNodeUUIDを渡してPrefabノードとの照合を維持し
        // a_isCloneSubtreeRoot = falseでルート専用後処理をスキップさせる
        l_child->Clone(l_cloneParent,
                       a_scene,
                       l_childPrefabHierarchyNodeUUID,
                       false);
    }
}

void FWK::GameObjectHierarchy::ConvertToPrefab(const boost::uuids::uuid& a_prefabUUID) const
{
    const auto& l_childList = m_childSmartPointerVectorList.GetREFElementDataList();

    for (const auto& l_childData : l_childList)
    {
        const auto& l_child = l_childData.m_type.lock();

        if (!l_child) { continue; }

        // 既に別のPrefabのインスタンスである子には伝播しない
        // その子は別のPrefabとして独自に更新伝播を受ける
        // PrefabUUIDに何かしら入っている場合はPrefab
        // なので処理を飛ばすこのゲームオブジェクトのプレハブの一部として扱わない
        if (const auto& l_childPrefabUUID = l_child->GetREFPrefabUUID();
            !l_childPrefabUUID.is_nil() &&
            l_childPrefabUUID != a_prefabUUID)
        {
            continue;
        }

        l_child->ConvertToPrefab(a_prefabUUID);
    }
}

void FWK::GameObjectHierarchy::DetachFromPrefab()
{
    // Prefab更新伝播用の削除追跡はPrefab由来ではなくなったため不要
    m_prefabRemovedChildUUIDSet.clear();
}

bool FWK::GameObjectHierarchy::ApplyParent(const std::weak_ptr<GameObject>& a_parent)
{
    const auto& l_owner  = m_owner.lock ();
    const auto& l_parent = a_parent.lock();

    FWK_ASSERT_RETURN_VALUE_IF(!l_owner, "GameObjectHierarchyのOwnerが無効なため、SetParentに失敗しました。", false);

    // 無効な親が渡されたらルート化と同じ意味にする
    if (!l_parent)
    {
        ClearParent();

        return true;
    }

    // 自分自身への親付けは禁止
    FWK_ASSERT_RETURN_VALUE_IF(l_parent == l_owner, "自分自身を親に指定することはできません。", false);

    // 既に同じ親なら何もしない
    if (m_parent.lock() == l_parent) { return true; }

    // 指定した親が自分の子孫なら循環になるので禁止
    FWK_ASSERT_RETURN_VALUE_IF(IsAncestorChainContainsOwner(a_parent), "子孫GameObjectを親に指定することはできません。", false);

    // 旧親の子リストから自分を外してローカル=ワールドへ戻す
    ClearParent();

    // 双方向リンクを張る
    auto& l_parentHierarchy = l_parent->GetMutableREFHierarchy();

    l_parentHierarchy.AddChild(m_owner);

    m_parent = a_parent;

    // Transformへ親を適用してワールド行列の整合を取る
    const auto& l_transform = l_owner->GetVALTransformComponent().lock();

    FWK_ASSERT_RETURN_VALUE_IF(!l_transform, "TransformComponentが無効なため、親Transformの適用に失敗しました。", false);

    l_transform->ApplyParent();

    return true;
}

void FWK::GameObjectHierarchy::ClearParent()
{
    const auto& l_owner = m_owner.lock ();
    
    if (const auto& l_parent = m_parent.lock();
        l_parent)
    {
        // 親側の子リストから自分を外す
        auto& l_parentHierarchy = l_parent->GetMutableREFHierarchy();

        l_parentHierarchy.RemoveChild(m_owner);
    }

    m_parent = {};

    if (!l_owner) { return; }

    const auto& l_transform = l_owner->GetVALTransformComponent().lock();

    // 親がいなくなったのでローカル = ワールドのStrategyへ戻す
    l_transform->ApplyStandalone();
}
void FWK::GameObjectHierarchy::ClearPrefabRemovedChildUUIDSet()
{
    m_prefabRemovedChildUUIDSet.clear();
}

void FWK::GameObjectHierarchy::ConnectParentForDeserialize(const std::weak_ptr<GameObject>& a_parent)
{
    const auto& l_owner  = m_owner.lock ();
    const auto& l_parent = a_parent.lock();
 
    if (!l_owner ||
        !l_parent)
    {
        return; 
    }
 
    // 親子リンクを直接設定する
    // ApplyParentと違いTransform::ApplyParentは呼ばない
    // TransformのMatrixUpdateModeは既にDeserializeで復元済みのため
    m_parent = a_parent;
 
    auto& l_parentHierarchy = l_parent->GetMutableREFHierarchy();
 
    l_parentHierarchy.AddChild(m_owner);
}

void FWK::GameObjectHierarchy::AddPrefabRemovedUUID(const boost::uuids::uuid& a_uuid)
{
    if (a_uuid.is_nil()) { return; }

    m_prefabRemovedChildUUIDSet.emplace(a_uuid);
}

void FWK::GameObjectHierarchy::AddChild(const std::weak_ptr<GameObject>& a_child)
{
    const auto& l_child = a_child.lock();

    if (!l_child) { return; }

    m_childSmartPointerVectorList.Add(a_child);

    // UUIDRegistryにも登録
    // PrefabHierarchyNodeUUIDがnilの場合は登録しない
    // （シーンで追加した子はnilなのでUUID管理対象外）
    auto l_uuid = l_child->GetREFPrefabHierarchyNodeUUID();

    if (l_uuid.is_nil()) 
    {
        l_uuid = GenerateVALChildUUID();

        l_child->SetPrefabHierarchyNodeUUID(l_uuid);
    }
    
    m_childUUIDRegistry.Add(l_child, l_uuid);
}

void FWK::GameObjectHierarchy::RemoveChild(const std::weak_ptr<GameObject>& a_child)
{
    const auto& l_child = a_child.lock();
 
    if (!l_child) { return; }

    auto l_uuid = l_child->GetREFPrefabHierarchyNodeUUID();
    
    if (l_uuid.is_nil()) { return; }

    // Prefab由来の子を削除した場合は削除済みUUIDとして記録する
    // Prefab更新伝播時に削除意図を保持するため
    // ClearParent経由の親変更も削除扱いになる
    if (l_child->GetVALIsPrefabOrigin())
    {
        m_prefabRemovedChildUUIDSet.emplace(l_uuid);
    }
 
    // UUIDRegistryからも削除
    m_childUUIDRegistry.Erase(l_uuid);
    
    m_childSmartPointerVectorList.RemoveSameElement(a_child);
}

bool FWK::GameObjectHierarchy::IsAncestorChainContainsOwner(const std::weak_ptr<GameObject>& a_gameObject) const
{
    const auto& l_owner   = m_owner.lock     ();
    auto        l_current = a_gameObject.lock();

    if (!l_owner) { return false; }

    // a_gameObjectから親を辿ってm_ownerに到達するならa_gameObjectはm_ownerの子孫
    while (l_current)
    {
        if (l_current == l_owner) { return true; }

        const auto& l_currentHierarchy = l_current->GetREFHierarchy     ();
        const auto& l_currentParent    = l_currentHierarchy.GetREFParent();

        l_current = l_currentParent.lock();
    }

    return false;
}

boost::uuids::uuid FWK::GameObjectHierarchy::GenerateVALChildUUID() const
{
    boost::uuids::uuid l_uuid        = {};
    auto&              l_uuidManager = Utility::UUIDManager::GetInstance();

    while (l_uuid.is_nil())
    {
        l_uuid = l_uuidManager.GenerateVALUUID();

        // m_childUUIDRegistryまたはm_prefabRemovedChildUUIDSetに含まれているUUID
        // なら意図的にnil値にしてもう一度UUIDを生成する
        if (m_childUUIDRegistry.Contains(l_uuid) ||
            m_prefabRemovedChildUUIDSet.contains(l_uuid))
        {
            l_uuid = {};
        }
    }

    return l_uuid;
}