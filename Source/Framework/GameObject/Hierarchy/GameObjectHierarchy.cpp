#include "GameObjectHierarchy.h"

void FWK::GameObjectHierarchy::INIT()
{
    m_parent = {};
    m_owner  = {};

    m_childSmartPointerVectorList.Clear();

    m_jsonConverter = {};
}

void FWK::GameObjectHierarchy::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

nlohmann::json FWK::GameObjectHierarchy::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

void FWK::GameObjectHierarchy::CloneTo(GameObjectHierarchy& a_cloneTarget, const std::weak_ptr<GameObject>& a_cloneOwner) const
{
    a_cloneTarget.m_owner = a_cloneOwner;
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

void FWK::GameObjectHierarchy::AddChild(const std::weak_ptr<GameObject>& a_child)
{
    m_childSmartPointerVectorList.Add(a_child);
}

void FWK::GameObjectHierarchy::RemoveChild(const std::weak_ptr<GameObject>& a_child)
{
    m_childSmartPointerVectorList.RemoveSameElement(a_child);
}