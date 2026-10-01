#include "ReparentGameObjectCommand.h"

FWK::Editor::ReparentGameObjectCommand::ReparentGameObjectCommand(boost::uuids::uuid a_gameObjectUUID, 
                                                                  boost::uuids::uuid a_beforeParentUUID,
                                                                  boost::uuids::uuid a_afterParentUUID) :
    m_gameObjectUUID  (std::move(a_gameObjectUUID)),
    m_beforeParentUUID(std::move(a_beforeParentUUID)),
    m_afterParentUUID (std::move(a_afterParentUUID))
{}
FWK::Editor::ReparentGameObjectCommand::~ReparentGameObjectCommand() = default;

void FWK::Editor::ReparentGameObjectCommand::Undo()
{
    // 変更前の親子関係へ戻す
    ApplyParent(m_beforeParentUUID);
}

void FWK::Editor::ReparentGameObjectCommand::Redo()
{
    // 変更後の親子関係へ進める
    ApplyParent(m_afterParentUUID);
}

void FWK::Editor::ReparentGameObjectCommand::ApplyParent(const boost::uuids::uuid& a_parentUUID)
{
    // SceneManager経由で現在のSceneを取得する
    auto& l_sceneManager = SceneManager::GetInstance();
    auto  l_scene        = l_sceneManager.GetVALScene().lock();
 
    if (!l_scene) { return; }
 
    // 対象GameObjectをUUIDから検索する
    auto l_gameObject = l_scene->FindVALGameObject(m_gameObjectUUID).lock();
 
    if (!l_gameObject ||
        l_gameObject->GetVALIsDestroyed())
    {
        return;
    }
 
    // 親GameObjectをUUIDから検索する
    // NilUUIDの場合はルート(親なし)を意味する
    std::weak_ptr<GameObject> l_parent = {};
 
    if (!a_parentUUID.is_nil())
    {
        l_parent = l_scene->FindVALGameObject(a_parentUUID);
 
        // 親が見つからない、または破棄済みの場合は処理を中断する
        const auto& l_parentLocked = l_parent.lock();
 
        if (!l_parentLocked ||
            l_parentLocked->GetVALIsDestroyed())
        {
            return;
        }
    }
 
    // Hierarchy::ApplyParentで親子関係を変更する
    // 循環参照チェックはApplyParent内部で行われる
    auto& l_hierarchy = l_gameObject->GetMutableREFHierarchy();
 
    l_hierarchy.ApplyParent(l_parent);
}