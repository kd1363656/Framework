#include "GameObjectHierarchy.h"

bool FWK::GameObjectHierarchy::ApplyParent(const std::weak_ptr<GameObject>& a_self, const std::weak_ptr<GameObject>& a_child)
{
    const auto& l_child = a_child.lock();

    if (!l_child) { return false; }

    const auto& l_childTransformComponent = l_child->GetVALTransformComponent().lock();

    FWK_ASSERT_RETURN_VALUE_IF(!l_childTransformComponent, "子GameObjectにTransformComponentが存在しません。", false);

    // 十八番関係の構築に失敗した場合はTransformへも適用しない
    if (!Parent(a_self, a_child)) { return false; }

    // TransformComponentへ新しい親GameObjectを適用する。
    l_childTransformComponent->ApplyParent(a_self);

    return true;
}
bool FWK::GameObjectHierarchy::Parent(const std::weak_ptr<GameObject>& a_self, const std::weak_ptr<GameObject>& a_child)
{
    const auto& l_self = a_self.lock();

    if (!l_self) { return false; }

    const auto& l_child = a_child.lock();

    if (!l_child) { return false; }

    // 自分自身を子にするとGameObjectの親子関係が即座に循環するため許可しない
    if (l_child == l_self ) { return false; }
    
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
    // Prefabを持たないGameObjectは確認対象外とする
    auto l_parent = l_self;

    // まずはRootからPrefabの重複があるかどうかを確認
    while (l_parent)
    {
        // unordered_set::emplace()の戻り値を利用して、
        // contains()とemplace()を別々に呼ばず
        // 一度の検索で重複確認と登録を行う
        if (const auto& l_prefabUUID = l_parent->GetREFPrefabUUID();
            !l_prefabUUID.is_nil() &&
            !l_prefabUUIDSet.emplace(l_prefabUUID).second)
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "親階層に同じPrefabUUIDを持つGameObjectが存在するため、親子関係を構築できませんでした。");

            return false;
        }

        const auto& l_parentHierarchy = l_parent->GetREFHierarchy();

        l_parent = l_parentHierarchy.GetREFParent().lock();
    }

    // 親階層だけでなく、
    // 新しく接続する子GameObject以下の階層も含めて
    // 同一路線上に同じPrefabUUIDが存在しないことを確認する
    if (ContainsDuplicatePrefabUUIDRecursive(a_child, l_prefabUUIDSet))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "同じ親子経路上に同じPrefabUUIDが存在するため、親子関係を構築できませんでした。");

        return false;
    }

          auto& l_childHierarchy = l_child->GetMutableREFHierarchy();
    const auto& l_currentParent  = l_childHierarchy.GetREFParent  ().lock();

    // すでに同じGameObjectが親の場合は二重登録しない
    if (l_currentParent == l_self) { return false; }

    // 現在の親GameObjectが存在する場合は
    // 新しい親を設定する前に親子関係を解除する
    if (l_currentParent)
    {
        auto& l_currentParentHierarchy = l_currentParent->GetMutableREFHierarchy();

        l_currentParentHierarchy.Unparent(l_currentParent, l_child);

        // Unparent()に失敗して現在の親が残っている場合は
        // 新しい親GameObjectを設定しない
        if (!l_childHierarchy.GetREFParent().expired())
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "以前の親GameObjectとの親子関係を解除できませんでした。");

            return false;
        }
    }

    // GameObject親子関係の構築
    l_childHierarchy.SetParent(l_self);

    // 親GameObject側へ子GameObjectを登録する
    m_childSmartPointerVectorList.Add(a_child);

    return true;
}
void FWK::GameObjectHierarchy::Unparent(const std::weak_ptr<GameObject>& a_self, const std::weak_ptr<GameObject>& a_child)
{
    const auto& l_self = a_self.lock();

    if (!l_self) { return; }

    const auto& l_child = a_child.lock();

    if (!l_child) { return; }

    auto& l_childHierarchy = l_child->GetMutableREFHierarchy();

    // 自分が親ではないGameObjectは解除しない
    if (const auto& l_currentParent = l_childHierarchy.GetREFParent().lock();
        l_currentParent != l_self)
    {
        return;
    }

    const auto& l_childTransformComponent = l_child->GetVALTransformComponent().lock();

    FWK_ASSERT_RETURN_IF(!l_childTransformComponent, "子GameObjectにTransformComponentが存在しないため、親子関係を解除できませんでした。");

    // Prefabの子スロットを外れる場合は削除記録へUUIDを追加し、
    // 子側のPrefabNodeUUIDは無効化する
    if (const auto& l_childNodeUUID = l_childHierarchy.GetREFPrefabNodeUUID();
        !l_childNodeUUID.is_nil())
    {
        m_removedChildNodeUUIDSet.emplace(l_childNodeUUID);
    }

    // 子の情報を削除し、子から親の情報を削除する
    m_childSmartPointerVectorList.RemoveSameElement(a_child);
    l_childHierarchy.ResetParent                   ();

    // 親がいなくても行列を計算できるようにする
    l_childTransformComponent->ApplyStandalone();
}

void FWK::GameObjectHierarchy::MarkChildForRemoval(const std::weak_ptr<GameObject>& a_child)
{
    const auto& l_child = a_child.lock();

    if (!l_child) { return; }

    auto& l_childHierarchy = l_child->GetMutableREFHierarchy();

    if (const auto& l_nodeUUID = l_child->GetREFPrefabUUID();
        !l_nodeUUID.is_nil())
    {
        m_removedChildNodeUUIDSet.emplace(l_nodeUUID);
    }
}

void FWK::GameObjectHierarchy::SweepRemovedChildren()
{
    std::vector<std::weak_ptr<GameObject>> l_markedList = {};

    const auto& l_childDataList = m_childSmartPointerVectorList.GetREFElementDataList();

    for (const auto& l_childData : l_childDataList)
    {
        if (const auto& l_child = l_childData.m_type.lock();
            l_child &&
            l_child->GetREFHierarchy().GetVALIsMarkedForUnparent())
        {
            l_markedList.emplace_back(l_childData.m_type);
        }
    }

    // 走査中の削除を避けるため、対象を集めてから一括で削除する
    for (const auto& l_childWeak : l_markedList)
    {
        const auto& l_child= l_childWeak.lock();

        if (!l_child) { continue; }

        const auto& l_childHierarchy = l_child->GetREFHierarchy();

        // 子の現在の親に解除処理を任せる
        if (const auto& l_parent = l_childHierarchy.GetREFParent().lock())
        {
            auto& l_parentHierarchy = l_parent->GetMutableREFHierarchy();

            l_parentHierarchy.Unparent(l_parent, l_child);
        }

        l_child->Destroy();
    }
}

void FWK::GameObjectHierarchy::ResetParent()
{
    m_parent.reset();
}

void FWK::GameObjectHierarchy::Clear()
{
    m_parent.reset();

    m_childSmartPointerVectorList.Clear();

    m_prefabNodeUUID = {};

    m_removedChildNodeUUIDSet.clear();

    m_isMarkedForUnparent = false;
}

bool FWK::GameObjectHierarchy::ContainsDuplicatePrefabUUIDRecursive(const std::weak_ptr<GameObject>& a_gameObject, std::unordered_set<boost::uuids::uuid>& a_prefabUUIDSet) const
{
    const auto& l_gameObject = a_gameObject.lock();

    if (!l_gameObject ||
        l_gameObject->GetVALIsDestroyed())
    {
        return false;
    }

    const auto& l_prefabUUID = l_gameObject->GetREFPrefabUUID();
    
    // Prefab情報を持っていないGameObjectは、
    // PrefabUUIDによる循環確認の対象には含めない
    if (l_prefabUUID.is_nil()) { return false; }

    // 現在確認している親子経路に
    // 同じPrefabUUIDが既に存在する場合は循環している
    if (!a_prefabUUIDSet.emplace(l_prefabUUID).second) { return true; }

    const auto& l_hierarchy                   = l_gameObject->GetREFHierarchy                      ();
    const auto& l_childSmartPointerVectorList = l_hierarchy.GetREFChildSmartPointerVectorList      ();
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

bool FWK::GameObjectHierarchy::IsDescendantOf(const std::shared_ptr<GameObject>& a_ancestor) const
{
    if (!a_ancestor) { return false; }

    auto l_parent = m_parent.lock();

    // 自身から親方向へさかのぼって
    // してGameObjectが祖先に存在するか確認する
    while (l_parent)
    {
        if (l_parent == a_ancestor) { return true; }

        const auto& l_parentHierarchy = l_parent->GetREFHierarchy();

        l_parent = l_parentHierarchy.GetREFParent().lock();
    }

    return false;
}