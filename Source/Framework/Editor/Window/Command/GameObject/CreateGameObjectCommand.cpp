#include "CreateGameObjectCommand.h"

FWK::Editor::CreateGameObjectCommand::CreateGameObjectCommand(std::vector<std::weak_ptr<GameObject>>&& a_createdGameObjectList, const boost::uuids::uuid& a_parentUUID) :
    m_parentUUID(a_parentUUID)
{
    // Undoでシーンから取り外しても実体が消えないようshared_ptrで保持する
    m_createdGameObjectList.reserve(a_createdGameObjectList.size());

    for (const auto& l_createdWeak : a_createdGameObjectList)
    {
        if (const auto& l_created = l_createdWeak.lock();
            l_created)
        {
            m_createdGameObjectList.emplace_back(l_created);
        }
    }
}
FWK::Editor::CreateGameObjectCommand::~CreateGameObjectCommand() = default;

void FWK::Editor::CreateGameObjectCommand::Undo()
{
    const auto& l_sceneManager = SceneManager::GetInstance ();
    const auto& l_scene        = l_sceneManager.GetVALScene().lock();
 
    if (!l_scene) { return; }
 
    for (const auto& l_created : m_createdGameObjectList)
    {
        if (!l_created) { continue; }
 
        // 破棄ではなくシーン管理から取り外す
        // 実体はコマンド側のshared_ptrが保持し続けるためRedoで復元できる
        l_scene->RemoveGameObject(l_created);
    }
}

void FWK::Editor::CreateGameObjectCommand::Redo()
{
    const auto& l_sceneManager = SceneManager::GetInstance ();
    const auto& l_scene        = l_sceneManager.GetVALScene().lock();
 
    if (!l_scene) { return; }
 
    // 生成時の親をUUIDから再解決する
    // nilまたは見つからなければルート(親なし)として再登録する
    const auto& l_parent = l_scene->FindVALGameObject(m_parentUUID).lock();
 
    for (const auto& l_created : m_createdGameObjectList)
    {
        if (!l_created) { continue; }
 
        // SceneInstanceUUIDが維持されているため
        // AddGameObjectで同一UUIDのままUUIDRegistryへ再登録される
        // もし同じSceneInstanceUUIDを持ったゲームオブジェクトがあった場合
        // SceneInstanceUUIDを変更してくれるので問題ない
        l_scene->AddGameObject(l_created);
 
        if (!l_parent) { continue; }
        
        auto& l_hierarchy = l_created->GetMutableREFHierarchy();

        l_hierarchy.ApplyParent(l_parent);
    }
}