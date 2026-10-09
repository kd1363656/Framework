#include "EditorGameObjectSelectionState.h"

void FWK::Editor::EditorGameObjectSelectionState::SelectSingleGameObject(const std::weak_ptr<GameObject>& a_gameObject)
{
    if (const auto& l_gameObject = a_gameObject;
        l_gameObject.expired())
    {
        return;
    }

    // 複数選択していた場合ように複数選択をクリアしてから
    // 一つのゲームオブジェクトをstd::vectorに格納する
    m_selectedGameObjectList.clear       ();
    m_selectedGameObjectList.emplace_back(a_gameObject);

    // 通常クリックでのみ範囲選択の開始地点を更新する
    // AssetBrowserEditorWindowFolderPane::SelectFolderと同じ規則
    m_rangeSelectionAnchor = a_gameObject;
}

void FWK::Editor::EditorGameObjectSelectionState::SelectGameObjectRange(const std::vector<std::weak_ptr<GameObject>>& a_gameObjectList)
{
    // 範囲選択は表示順に並んだ範囲をそのまま選択状態へ置き換える
    // アンカーは更新しない(開始地点が動くと連続したShift選択ができなくなるため)
    m_selectedGameObjectList.clear();

    for (const auto& l_gameObject : a_gameObjectList)
    {
        if (l_gameObject.expired()) { continue; }

        m_selectedGameObjectList.emplace_back(l_gameObject);
    }
}

void FWK::Editor::EditorGameObjectSelectionState::ToggleSelectedGameObject(const std::weak_ptr<GameObject>& a_gameObject)
{
    if (const auto& l_gameObject = a_gameObject;
        l_gameObject.expired())
    {
        return;
    }

    // Ctrl + クリックで選択済みなら解除、未選択なら追加
    // アンカーは更新しない(AssetBrowserと同じ規則)
    if (FindVALIsSelected(a_gameObject))
    {
        RemoveSelectedGameObject(a_gameObject);

        return;
    }

    m_selectedGameObjectList.emplace_back(a_gameObject);
}

void FWK::Editor::EditorGameObjectSelectionState::AddSelectedGameObject(const std::weak_ptr<GameObject>& a_gameObject)
{
    const auto& l_gameObject = a_gameObject.lock();

    if (!l_gameObject) { return; }

    // 既に選択済みなら追加しない
    if (FindVALIsSelected(a_gameObject)) { return; }

    m_selectedGameObjectList.emplace_back(a_gameObject);
}

void FWK::Editor::EditorGameObjectSelectionState::RemoveSelectedGameObject(const std::weak_ptr<GameObject>& a_gameObject)
{
    const auto& l_gameObject = a_gameObject.lock();

    if (!l_gameObject) { return; }

    // weak_ptr同士は直接比較できないため
    // lock()したshared_ptrのアドレスで同一性を判定する
    std::erase_if(m_selectedGameObjectList,
                  [&l_gameObject](const auto& a_selectedGameObjectWeak)
                  {
                      return a_selectedGameObjectWeak.lock() == l_gameObject;
                  });
}

void FWK::Editor::EditorGameObjectSelectionState::ClearSelectedGameObjectList()
{
    m_selectedGameObjectList.clear();

    m_rangeSelectionAnchor = {};
}

void FWK::Editor::EditorGameObjectSelectionState::SweepUnavailableGameObjects()
{
    // 破棄済み、参照切れのGameObjectを選択状態から取り除く
    // Undoで生成が取り消されたGameObjectはコマンドがshared_ptrを保持し続けるため
    // 実体が消えずlock()も成功するが、SceneのUUIDRegistryからは外れている
    // 「シーン管理下にあるか」をRegistry解決で判定して選択対象から外す
    const auto& l_sceneManager = SceneManager::GetInstance ();
    const auto& l_scene        = l_sceneManager.GetVALScene().lock();

    std::erase_if(m_selectedGameObjectList,
                  [&l_scene](const auto& a_gameObjectWeak)
                  {
                      const auto& l_gameObject = a_gameObjectWeak.lock();

                      // Destroy()されたGameObjectはScene::EarlyUpdateでリストから外れるまで
                      // 生存しているためexpiredだけでなくIsDestroyedも判定対象にする
                      if (!l_gameObject ||
                          l_gameObject->GetVALIsDestroyed())
                      {
                          return true;
                      }

                      // Sceneそのものが無ければ全て無効扱い
                      if (!l_scene) { return true; }

                      // シーンのUUIDRegistryで自分自身が解決できるか確認する
                      // 取り外されたGameObjectはRegistryから消えているため
                      // 同一UUIDで検索しても自分が返らない
                      return l_scene->FindVALGameObject(l_gameObject->GetREFSceneInstanceUUID()).lock() != l_gameObject;
                  });

    // アンカーも同じ判定で確認する
    if (const auto& l_anchor = m_rangeSelectionAnchor.lock();
        !l_anchor ||
        !l_scene ||
        l_anchor->GetVALIsDestroyed() ||
        l_scene->FindVALGameObject(l_anchor->GetREFSceneInstanceUUID()).lock() != l_anchor)
    {
        m_rangeSelectionAnchor = {};
    }
}

void FWK::Editor::EditorGameObjectSelectionState::RestoreState(const std::vector<boost::uuids::uuid>& a_uuidList, const boost::uuids::uuid& a_anchorUUID)
{
    // SceneManager経由で現在のSceneを取得する
    const auto& l_sceneManager = SceneManager::GetInstance ();
    const auto& l_scene        = l_sceneManager.GetVALScene().lock();

    if (!l_scene) { return; }

    // 選択リストをクリアしてからUUIDリストから再構築する
    m_selectedGameObjectList.clear  ();
    m_selectedGameObjectList.reserve(a_uuidList.size());

    for (const auto& l_uuid : a_uuidList)
    {
        // nil UUIDは無視する
        if (l_uuid.is_nil()) { continue; }

        const auto& l_gameObject = l_scene->FindVALGameObject(l_uuid).lock();

        // 破棄済みまたは存在しないGameObjectは選択対象から外す
        if (!l_gameObject ||
            l_gameObject->GetVALIsDestroyed())
        {
            continue;
        }

        m_selectedGameObjectList.emplace_back(l_gameObject);
    }

    // アンカーUUIDからGameObjectを検索して設定する
    if (!a_anchorUUID.is_nil())
    {
        m_rangeSelectionAnchor = l_scene->FindVALGameObject(a_anchorUUID);
    }
    else
    {
        m_rangeSelectionAnchor = {};
    }
}

std::weak_ptr<FWK::GameObject> FWK::Editor::EditorGameObjectSelectionState::FindVALLastSelectedGameObject() const
{
    if (m_selectedGameObjectList.empty()) { return {}; }

    return m_selectedGameObjectList.back();
}

bool FWK::Editor::EditorGameObjectSelectionState::FindVALIsSelected(const std::weak_ptr<GameObject>& a_gameObject) const
{
    const auto& l_gameObject = a_gameObject.lock();

    if (!l_gameObject) { return false; }

    // std::ranges::anu_ofでリスト内に同じアドレスを持つ要素があるか確認する
    return std::ranges::any_of(m_selectedGameObjectList,
                               [&l_gameObject](const auto& a_selectedGameObjectWeak)
                               {
                                   return a_selectedGameObjectWeak.lock() == l_gameObject;
                               });
}