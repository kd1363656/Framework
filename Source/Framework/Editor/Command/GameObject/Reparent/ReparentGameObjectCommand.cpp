#include "ReparentGameObjectCommand.h"

FWK::Editor::ReparentGameObjectCommand::ReparentGameObjectCommand(const boost::uuids::uuid& a_gameObjectUUID, const Struct::ReparentGameObjectState& a_beforeState, const Struct::ReparentGameObjectState& a_afterState) :
    m_gameObjectUUID(a_gameObjectUUID),

    m_beforeState(a_beforeState),
    m_afterState (a_afterState)
{}
FWK::Editor::ReparentGameObjectCommand::~ReparentGameObjectCommand() = default;

void FWK::Editor::ReparentGameObjectCommand::Undo()
{
    // 変更前の親と兄弟の並びへ戻す
    ApplyState(m_beforeState);
}

void FWK::Editor::ReparentGameObjectCommand::Redo()
{
    // 変更後の親と兄弟の並びへ進める
    ApplyState(m_afterState);
}

void FWK::Editor::ReparentGameObjectCommand::ApplyState(const Struct::ReparentGameObjectState& a_state)
{
    // SceneManager経由で現在のSceneを取得する
    const auto& l_sceneManager = SceneManager::GetInstance ();
    const auto& l_scene        = l_sceneManager.GetVALScene().lock();

    if (!l_scene) { return; }

    // 対象GameObjectをUUIDから検索する
    const auto& l_gameObject = l_scene->FindVALGameObject(m_gameObjectUUID).lock();

    if (!l_gameObject ||
        l_gameObject->GetVALIsDestroyed())
    {
        return;
    }

    // 親GameObjectをUUIDから検索する
    // NilUUIDの場合はルート(親なし)を意味する
    std::weak_ptr<GameObject> l_parentWeak = {};

    if (!a_state.m_parentUUID.is_nil())
    {
        l_parentWeak = l_scene->FindVALGameObject(a_state.m_parentUUID);

        // 親が見つからない、または破棄済みの場合は処理を中断する
        const auto& l_parentLocked = l_parentWeak.lock();

        if (!l_parentLocked ||
            l_parentLocked->GetVALIsDestroyed())
        {
            return;
        }
    }

    // Hierarchy::ApplyParentで親子関係を変更する
    // 循環参照チェックはApplyParent内部で行われる
    // 親が変わる場合は親の子リストの末尾へ追加される
    if (auto& l_hierarchy = l_gameObject->GetMutableREFHierarchy();
        !l_hierarchy.ApplyParent(l_parentWeak))
    {
        return;
    }

    // 記録しておいた兄弟のUUIDの並びで、親の子リスト(親がいなければシーンのルート)を並べ直す
    // 位置を添字で持たず、UUIDから並びを組み直すためリストが多少変わっていても位置がずれない
    Utility::ApplyGameObjectSiblingOrder(l_parentWeak, a_state, *l_scene);

    // 階層の深さや並びが変わったため実行レベルを再構築する
    l_scene->RebuildGameObjectExecutionLevelList();
}