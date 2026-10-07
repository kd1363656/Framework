#include "DestroyGameObjectCommand.h"

FWK::Editor::DestroyGameObjectCommand::DestroyGameObjectCommand(std::vector<Struct::DestroyedGameObjectRecord>&& a_destroyedGameObjectRecordList) :
    m_destroyedGameObjectRecordList(std::move(a_destroyedGameObjectRecordList))
{}
FWK::Editor::DestroyGameObjectCommand::~DestroyGameObjectCommand() = default;

void FWK::Editor::DestroyGameObjectCommand::Undo()
{
    const auto& l_sceneManager = SceneManager::GetInstance ();
    const auto& l_scene        = l_sceneManager.GetVALScene().lock();

    if (!l_scene) { return; }

    for (const auto& l_record : m_destroyedGameObjectRecordList)
    {
        const auto& l_subtreeList = l_record.m_detachedSubtreeGameObjectList;

        // 親→子の順に並んでいるためその順でシーンへ戻す
        // 親子関係は取り外し時も維持しているため再接続は不要
        for (const auto& l_gameObject : l_subtreeList)
        {
            l_scene->AddGameObject(l_gameObject);
        }

        // 取り外したルートの元の親をUUIDから再解決する
        // nilまたは見つからなければルート(親なし)のままにする
        const auto& l_parent = l_scene->FindVALGameObject(l_record.m_parentUUID).lock();

        if (!l_parent) { continue; }

        // 先頭が取り外したルート
        const auto& l_root = l_subtreeList.front();

        auto& l_rootHierarchy = l_root->GetMutableREFHierarchy();

        l_rootHierarchy.ApplyParent(l_parent);

        // 取り外した時にPrefab由来の子は親へ削除済みとして記録されているため
        // 戻したので記録を消す(消さないとPrefab更新伝播で削除扱いのままになる)
        auto& l_parentHierarchy = l_parent->GetMutableREFHierarchy                        ();
        auto& l_removedUUIDSet  = l_parentHierarchy.GetMutableREFPrefabRemovedChildUUIDSet();

        l_removedUUIDSet.erase(l_root->GetREFPrefabHierarchyNodeUUID());
    }
}
void FWK::Editor::DestroyGameObjectCommand::Redo()
{
    const auto& l_sceneManager = SceneManager::GetInstance ();
    const auto& l_scene        = l_sceneManager.GetVALScene().lock();

    if (!l_scene) { return; }

    for (const auto& l_record : m_destroyedGameObjectRecordList)
    {
        // 破棄ではなくシーン管理から取り外す
        // 実体はコマンド側のshared_ptrが保持し続けるためUndoで復元できる
        // 子孫の親子関係は維持したまま、ルートだけ親から外れる
        Utility::DetachGameObjectSubtree(l_record.m_detachedSubtreeGameObjectList, *l_scene);
    }
}