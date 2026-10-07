#include "CreateGameObjectCommand.h"

FWK::Editor::CreateGameObjectCommand::CreateGameObjectCommand(const std::vector<std::weak_ptr<GameObject>>& a_createdGameObjectList)
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

    // Undo時点の子孫を集め直す
    // 生成後に子孫が変わっている可能性があるためコンストラクタではなくここで集める
    m_descendantGameObjectList.clear();

    for (const auto& l_created : m_createdGameObjectList)
    {
        Utility::CollectDescendantGameObjectList(l_created, m_descendantGameObjectList);
    }

    // 子孫はシーン管理から外すだけで親子関係は維持する
    // Redoでルートを戻したときに子孫の階層をそのまま復元するため
    for (const auto& l_descendant : m_descendantGameObjectList)
    {
        l_scene->UnregisterGameObject(l_descendant);
    }

    // Undo時点の親を記録し直す
    // 複製のようにルートごとに親が異なる場合があるため1つずつ記録する
    m_parentUUIDList.clear  ();
    m_parentUUIDList.reserve(m_createdGameObjectList.size());

    // m_createdGameObjectListはコンストラクタで有効なものだけを保持しているためnullチェックは不要
    // m_parentUUIDListと添字を揃えるためにも全要素を処理する
    for (const auto& l_created : m_createdGameObjectList)
    {
        // RemoveGameObjectで親子関係が解除されるため取り外す前に親を記録する
        // 親がいなければルートとしてnilを記録する
        const auto& l_hierarchy = l_created->GetREFHierarchy();
        const auto& l_parent    = l_hierarchy.GetREFParent  ().lock();

        m_parentUUIDList.emplace_back(l_parent ? l_parent->GetREFSceneInstanceUUID() : boost::uuids::uuid{});

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
 
    // m_parentUUIDListと添字を揃えるため添字でループする
    for (std::size_t l_i = 0ULL; l_i < m_createdGameObjectList.size(); ++l_i)
    {
        const auto& l_created = m_createdGameObjectList[l_i];

        // SceneInstanceUUIDが維持されているため
        // AddGameObjectで同一UUIDのままUUIDRegistryへ再登録される
        // もし同じSceneInstanceUUIDを持ったゲームオブジェクトがあった場合
        // SceneInstanceUUIDを変更してくれるので問題ない
        l_scene->AddGameObject(l_created);

        // Undo時に記録した親をUUIDから再解決する
        // nilまたは見つからなければルート(親なし)のままにする
        const auto& l_parent = l_scene->FindVALGameObject(m_parentUUIDList[l_i]).lock();

        if (!l_parent) { continue; }
        
        auto& l_hierarchy = l_created->GetMutableREFHierarchy();

        l_hierarchy.ApplyParent(l_parent);
    }

    // 子孫は親→子の順に集めているためその順で再登録する
    // 親子関係はUndo時に維持しているため再接続は不要
    for (const auto& l_descendant : m_descendantGameObjectList)
    {
        // 同じSceneInstanceUUIDをシーンから検出した場合自動で再発行してくれるので問題なし
        l_scene->AddGameObject(l_descendant);
    }
}