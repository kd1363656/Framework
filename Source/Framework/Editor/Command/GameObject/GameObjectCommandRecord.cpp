#include "GameObjectCommandRecord.h"

// GameObjectを作成・削除するコマンドで使う記録
// GameObjectの実体(shared_ptr)は持たず、UUID・シーンに保存するときと同じ形のJson・場所(親と兄弟の並び)で覚える
// 実体を持つと、シーンから外した後もコンポーネントが生きたまま残り(物理のBodyなど)、
// 履歴の上限まで溜まった実体のメモリも減らない
// Jsonなら外したときに実体ごと解放され、戻すときはシーンの読み込みと同じ手順で作り直せる
void FWK::Editor::GameObjectCommandRecord::Capture(const std::weak_ptr<GameObject>& a_gameObject, Scene& a_scene)
{
    const auto& l_gameObject = a_gameObject.lock();

    if (!l_gameObject) { return; }

    m_gameObjectUUID = l_gameObject->GetREFSceneInstanceUUID();
    m_placement      = Utility::FetchVALGameObjectPlacement (l_gameObject, a_scene);

    auto& l_prefabSystem = a_scene.GetMutableREFGameObjectPrefabSystem();

    m_prefabNodeJson = FetchVALPrefabNodeJson(*l_gameObject, l_prefabSystem);

    // Prefabの中の子はPrefabの対応するノードとの差分、それ以外はシーンの保存と同じ形(Prefab由来なら差分、そうでなければ全部)で覚える
    // 子孫はChildListの中に一緒に入る
    // SerializeSceneにPrefabの中の子を渡すとPrefabのルートとの差分になり、戻すときにルートの値と混ざってしまうため分けている
    // 例 : Prefab P { 親(0, 0, 0) - 子(1, 0, 0) } の子を消す
    //      → 子のノードとの差分 { }(変更なし)を覚え、戻すときはノード + 差分で(1, 0, 0)に戻る
    if (m_prefabNodeJson.is_null())
    {
        m_json = l_gameObject->SerializeScene(l_prefabSystem);

        return;
    }

    m_json = l_gameObject->SerializeDIFF(m_prefabNodeJson, l_prefabSystem);
}

void FWK::Editor::GameObjectCommandRecord::Remove(Scene& a_scene) const
{
    // UUIDで探して、シーンに無ければ何もしない
    // 同じ記録のRemoveが2回呼ばれても壊れないようにするため
    const auto& l_gameObject = a_scene.FindVALGameObject(m_gameObjectUUID).lock();

    if (!l_gameObject) { return; }

    // 破棄フラグ(GameObject::Destroy)は使わず、子孫ごとシーンの一覧から外す
    // この記録は実体を持っていないため、外した後に誰も持っていなければここで解放される
    a_scene.RemoveGameObjectTree(l_gameObject);
}
void FWK::Editor::GameObjectCommandRecord::Restore(Scene& a_scene) const
{
    // Jsonが無い、または既にシーンにあるなら何もしない
    // 作成直後のGameObjectはシーンにあるため、作成のRedoを最初に呼んでも二重に作られない
    if (m_json.is_null() ||
        IsInScene(a_scene))
    {
        return;
    }

    // 親をUUIDで探す(nilはシーンのルート)
    // 親が先に消されているなら戻す場所が無いため戻さない(親を戻すUndoの方が先に来る)
    std::shared_ptr<GameObject> l_parent = {};

    if (!m_placement.m_parentUUID.is_nil())
    {
        l_parent = a_scene.FindVALGameObject(m_placement.m_parentUUID).lock();

        if (!l_parent ||
            l_parent->GetVALIsDestroyed())
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "戻す先の親GameObjectがシーンに無いため、GameObjectを戻せませんでした。\nUUID : {}", boost::uuids::to_string(m_gameObjectUUID));

            return;
        }
    }

    const auto& l_gameObject = std::make_shared<GameObject>();

    l_gameObject->INIT();

    // シーンの読み込みと同じ手順でJsonから作り直す
    // SceneInstanceUUIDもJsonから戻るため、他のコマンドが覚えているUUIDで同じ物を探せる
    // 子孫はChildListから再帰的に作られ、シーンへの登録(保留リスト)もこの中で行われる
    const auto& l_prefabSystem = a_scene.GetREFGameObjectPrefabSystem();

    l_gameObject->DeserializeScene(m_json,
                                   m_prefabNodeJson,
                                   l_prefabSystem,
                                   a_scene);

    if (l_parent)
    {
        // 消したときにRemoveChildが親へ残した「Prefabから消した子」の記録を消す
        // 残したままだとシーン保存でPrefabとの差分に「消した子」として書かれ、読み直すと消えてしまうため
        auto& l_parentHierarchy           = l_parent->GetMutableREFHierarchy                        ();
        auto& l_prefabRemovedChildUUIDSet = l_parentHierarchy.GetMutableREFPrefabRemovedChildUUIDSet();

        l_prefabRemovedChildUUIDSet.erase(l_gameObject->GetREFPrefabHierarchyNodeUUID());

        // Jsonのローカル値は消す前の親を基準にした値なので、読み込み用の接続で親につなぐ
        // ApplyParentだとローカル値をワールド値とみなしてローカルを計算し直すため位置がずれる
        // 例 : 親(10, 0, 0)の子のローカル(1, 0, 0) → ApplyParentだと(1, 0, 0)をワールドとみなしローカルが(-9, 0, 0)になる
        auto& l_hierarchy = l_gameObject->GetMutableREFHierarchy();

        l_hierarchy.ConnectParentForDeserialize(l_parent);
    }

    // 保留リストから一覧へすぐ反映する
    // AddGameObjectは保留リストに入れるだけで、一覧に入るのは次のフレームのEarlyUpdateのため
    // 実行レベルは親を辿って決まるため、親につないだ後に行う
    a_scene.AddPendingGameObjects();

    // 行列の計算方法(親基準かどうか)などポインタのつなぎ直し
    l_gameObject->PostDeserialize();

    // 兄弟の並びを記録どおりに組み直す
    Utility::ApplySiblingOrder(m_placement.m_siblingUUIDList, l_parent, a_scene);
}

bool FWK::Editor::GameObjectCommandRecord::IsInScene(const Scene& a_scene) const
{
    // シーンのUUID登録から引けるならシーンに入っている
    return !a_scene.FindVALGameObject(m_gameObjectUUID).expired();
}

nlohmann::json FWK::Editor::GameObjectCommandRecord::FetchVALPrefabNodeJson(const GameObject& a_gameObject, const SceneGameObjectPrefabSystem& a_prefabSystem) const
{
    // Prefabの中の子(親と同じPrefab由来)でなければ、差分の元になるノードは無い
    if (!a_gameObject.FetchVALIsPrefabInternalChild()) { return {}; }

    const auto* l_prefab = a_prefabSystem.FindPTRPrefab(a_gameObject.GetREFPrefabUUID());

    if (!l_prefab) { return {}; }

    // PrefabのJsonを子へ辿り、同じPrefabHierarchyNodeUUIDのノードを探す
    const auto* l_nodeJson = Converter::GameObjectJsonConverter::FindPTRPrefabNodeJson(l_prefab->GetREFJson(), a_gameObject.GetREFPrefabHierarchyNodeUUID());

    if (!l_nodeJson) { return {}; }

    return *l_nodeJson;
}