#include "GameObject.h"

void FWK::GameObject::INIT()
{
    if (!m_transformComponent)
    {
        m_transformComponent = std::make_shared<GameObjectTransformComponent>();
    }

    m_transformComponent->SetOwner(weak_from_this());

    m_hierarchy.INIT    ();
    m_hierarchy.SetOwner(weak_from_this());

    m_componentContainer.INIT    ();
    m_componentContainer.SetOwner(weak_from_this());

    m_jsonConverter = {};

    m_name = {};

    m_prefabUUID              = {};
    m_prefabHierarchyNodeUUID = {};
    m_sceneInstanceUUID       = {};

    m_isDestroyed    = false;
    m_isPrefabOrigin = Constant::k_gameObjectInitialValueIsPrefabOrigin;
}

void FWK::GameObject::DeserializeScene(const nlohmann::json&              a_rootJson, 
                                       const nlohmann::json&              a_baseJson, 
                                       const SceneGameObjectPrefabSystem& a_prefabSystem, 
                                             Scene&                       a_scene)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.DeserializeScene(weak_from_this(), 
                                     a_rootJson,
                                     a_baseJson, 
                                     a_prefabSystem, 
                                     a_scene);
}
void FWK::GameObject::DeserializePrefab(const nlohmann::json& a_rootJson, const SceneGameObjectPrefabSystem& a_prefabSystem, Scene& a_scene)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.DeserializePrefab(weak_from_this(),
                                      a_rootJson,
                                      a_prefabSystem,
                                      a_scene);
}

void FWK::GameObject::PostDeserialize()
{
    if (m_transformComponent) 
    {
        m_transformComponent->PostDeserialize();
    }

    m_componentContainer.PostDeserialize();

    // このゲームオブジェクトと子ゲームオブジェクトを再帰的に
    // PostDeserialize()処理を行う
    m_hierarchy.PostDeserialize();
}

void FWK::GameObject::EarlyUpdate() const
{
    m_componentContainer.EarlyUpdate();
}
void FWK::GameObject::Update() const
{
    m_componentContainer.Update();
}
void FWK::GameObject::LateUpdate() const
{
    m_componentContainer.LateUpdate();
}
void FWK::GameObject::PostLateUpdate() const
{
    if (m_transformComponent)
    {
        m_transformComponent->PostLateUpdate();
    }

    m_componentContainer.PostLateUpdate();
}

nlohmann::json FWK::GameObject::Serialize(SceneGameObjectPrefabSystem& a_prefabSystem) const
{
    return m_jsonConverter.Serialize(*this, a_prefabSystem);
}
nlohmann::json FWK::GameObject::SerializeScene(SceneGameObjectPrefabSystem& a_prefabSystem) const
{
    return m_jsonConverter.SerializeScene(*this, a_prefabSystem);
}
nlohmann::json FWK::GameObject::SerializeDIFF(const nlohmann::json& a_baseJson, SceneGameObjectPrefabSystem& a_prefabSystem) const
{
    return m_jsonConverter.SerializeDIFF(a_baseJson, *this, a_prefabSystem);
}

void FWK::GameObject::Destroy()
{
    m_isDestroyed = true;

    m_hierarchy.Destroy();
}

std::shared_ptr<FWK::GameObject> FWK::GameObject::Clone(const std::weak_ptr<GameObject>& a_newParent,
                                                              Scene&                     a_scene, 
                                                        const boost::uuids::uuid&        a_prefabHierarchyNodeUUID, 
                                                              bool                       a_isCloneSubtreeRoot) const
{
    // 破棄済みのGameObjectはクローンしない
    if (m_isDestroyed)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "破棄済みのGameObjectのため、クローンを中断しました。");

        return nullptr;
    }

    const auto& l_newParent = a_newParent.lock();
 
    // 指定された親が破棄済みだと接続してもすぐに消えるため中断する
    if (l_newParent &&
        l_newParent->GetVALIsDestroyed())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "指定された親GameObjectが破棄済みのため、クローンを中断しました。");
 
        return nullptr;
    }
 
    auto l_clone = std::make_shared<GameObject>();
 
    // Owner設定等の初期化
    l_clone->INIT();
 
    // PrefabHierarchyNodeUUIDは接続前に設定する
    // クローンルート呼び出し時はNilのまま渡されるため親がいればAddChildが新規発行する
    // 子孫の再帰呼び出しではコピー元のUUIDが渡されPrefabノードとの照合が維持される
    l_clone->SetPrefabHierarchyNodeUUID(a_prefabHierarchyNodeUUID);
 
    // 親がいるなら接続する
    // ApplyParentはローカルSRTをワールド維持で再計算するためここでは使わない
    if (l_newParent)
    {
        auto& l_cloneHierarchy = l_clone->GetMutableREFHierarchy();

        // 親とクローン側ヒエラルキーの親子関係を構築する
        l_cloneHierarchy.ConnectParentForDeserialize(l_newParent);
    }
 
    // メンバの値コピー
    // SceneInstanceUUID / IsDestroyed は複写しない
    l_clone->m_name           = m_name;
    l_clone->m_prefabUUID     = m_prefabUUID;
    l_clone->m_isPrefabOrigin = m_isPrefabOrigin;
 
    // TransformComponentはSerialize往復による値コピー
    // Ownerはクローン側のGameObjectへ張り替える
    if (m_transformComponent)
    {
        l_clone->m_transformComponent = m_transformComponent->Clone();
 
        if (l_clone->m_transformComponent)
        {
            l_clone->m_transformComponent->SetOwner(l_clone->weak_from_this());
        }
    }
 
    // ComponentContainerが自身のメンバをコピーする
    m_componentContainer.Clone(l_clone->m_componentContainer);
 
    // Sceneへ登録(SceneInstanceUUIDはNilなのでここで新規発行される)
    // 実行レベル計算に親子関係が要るため子のクローンより先に行う
    a_scene.AddGameObject(l_clone);
 
    // 子を再帰的にクローンする
    m_hierarchy.Clone(l_clone->GetMutableREFHierarchy(), a_scene);
 
    // ここから先はクローンのルートのみ行う後処理
    if (!a_isCloneSubtreeRoot) { return l_clone; }
 
    // コピー元がPrefab内部の子だった場合はクローンをPrefabから剥がす
    // PrefabUUIDが残るとSerializeSceneがPrefabルートJsonとの誤った差分を作るため
    // 子孫も同じPrefabUUIDを持つものだけ再帰的に剥がれる(別Prefabのネストは維持)
    if (FetchVALIsPrefabInternalChild())
    {
        l_clone->DetachFromPrefab();
    }
 
    // サブツリー全体のPostDeserializeを親→子順で行う
    // TransformのMatrixUpdateModeや各ComponentのPostDeserializeが
    // 親子接続済みの状態を必要とするため構築完了後に呼ぶ
    l_clone->PostDeserialize();
 
    // クローンルートのTransformModeを親の有無に合わせる
    // 親なし : Hierarchicalのままだと意味的におかしいためStandaloneへ切り替える
    // 親あり : Standaloneのままだと親に追従しないためHierarchicalへ切り替えワールドを維持する
    if (const auto& l_cloneTransform = l_clone->GetVALTransformComponent().lock())
    {
        // 親がいるなら親基準で行列を確定、そうでないなら独立して行列を確定する
        if (l_newParent)
        {
            l_cloneTransform->ApplyParent();
        }
        else
        {
            l_cloneTransform->ApplyStandalone();
        }
    }
 
    return l_clone;
}

void FWK::GameObject::ConvertToPrefab(const boost::uuids::uuid& a_prefabUUID)
{
    // 自身のプレハブ情報を設定する
    m_prefabUUID     = a_prefabUUID;
    m_isPrefabOrigin = true;
 
    // PrefabHierarchyNodeUUIDがnilなら新規生成する
    // プレハブ内での識別に使うため必須
    if (m_prefabHierarchyNodeUUID.is_nil())
    {
        auto& l_uuidManager = Utility::UUIDManager::GetInstance();
 
        m_prefabHierarchyNodeUUID = l_uuidManager.GenerateVALUUID();
    }
 
    // コンポーネントコンテナのコンポーネントもプレハブ化
    m_componentContainer.ConvertToPrefab();

    // 子も再帰的にプレハブ化する
    m_hierarchy.ConvertToPrefab(a_prefabUUID);
}

void FWK::GameObject::DetachFromPrefab()
{
    // 子への再帰判定で使うため、先に元のPrefabUUIDを保持する
    const auto l_oldPrefabUUID = m_prefabUUID;
 
    // Prefabとの紐付けを断つ
    // 以降このGameObjectはPrefab由来ではない通常のGameObjectとして
    // フル形式でシリアライズされる
    m_prefabUUID     = {};
    m_isPrefabOrigin = Constant::k_gameObjectInitialValueIsPrefabOrigin;
 
    // PrefabHierarchyNodeUUIDはあえて変更しない
    // 既に親のChildUUIDRegistryへこのUUIDで登録済みであり
    // 変更するとRegistryのキーと不整合になる
 
    // ComponentContainerとHierarchyが持つPrefab追跡情報も剥がす
    m_componentContainer.DetachFromPrefab();
    m_hierarchy.DetachFromPrefab         (l_oldPrefabUUID);
}

void FWK::GameObject::ClearAllPrefabRemovedUUIDSet()
{
    // 自身のHierarchyの削除済み子UUID集合をクリアする
    m_hierarchy.ClearPrefabRemovedChildUUIDSet();

    // 自身のComponentContainerの削除済みコンポーネントUUID集合をクリアする
    m_componentContainer.ClearPrefabRemovedComponentUUIDSet();
}

bool FWK::GameObject::FetchVALIsPrefabInternalChild() const
{
    // PrefabUUIDが無い = Prefab由来ではないので内部子ではない
    if (m_prefabUUID.is_nil()) { return false; }

    const auto& l_parent = m_hierarchy.GetREFParent().lock();

    // 親がいない = Prefabインスタンスのルートか通常GameObject
    if (!l_parent) { return false; }

    // 親が同じPrefabUUIDを持つ = このGameObjectはPrefab内部の子
    // 親のPrefabUUIDが違う     = Prefabインスタンスのルート
    return l_parent->GetREFPrefabUUID() == m_prefabUUID;
}