#include "Scene.h"
#include "../../Application/Application.h"

void FWK::Scene::INIT()
{
    m_gameObjectList.clear              ();
    m_gameObjectExecutionLevelList.clear();

    m_gameObjectUUIDRegistry.Clear();

    m_prefabSystem.INIT();

    m_assetFilePathRegistry.INIT();

    m_lightSystem.ApplyDefaultSettings();

    m_sceneName.clear();

    m_nextSceneUUID = {};
}

void FWK::Scene::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "Jsonの中身が空となっておりシーンのデシリアライズ処理に失敗しました。");
        return;
    }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::Scene::PostDeserialize() const
{
    for (const auto& l_gameObject : m_gameObjectList)
    {
        if (!l_gameObject ||
            l_gameObject->GetVALIsDestroyed())
        {
            continue;
        }

        l_gameObject->PostDeserialize();
    }

    auto& l_physicsManager = Physics::PhysicsManager::GetInstance();

    // StaticBodyをまとめて追加した後なので、BroadPhaseを最適化しておく。
    // 毎フレーム呼ぶものではなく、ステージ読み込み後などに呼ぶ。
    l_physicsManager.OptimizeBroadPhase();
}

void FWK::Scene::EarlyUpdate()
{
    for (const auto& l_gameObjectExecutionLevel : m_gameObjectExecutionLevelList)
    {
        for (const auto& l_gameObjectWeak : l_gameObjectExecutionLevel)
        {
            const auto& l_gameObject = l_gameObjectWeak.lock();

            if (!l_gameObject ||
                l_gameObject->GetVALIsDestroyed())
            {
                continue;
            }

            l_gameObject->EarlyUpdate();
        }
    }
}
void FWK::Scene::Update() const
{
    for (const auto& l_gameObjectExecutionLevel : m_gameObjectExecutionLevelList)
    {
        for (const auto& l_gameObjectWeak : l_gameObjectExecutionLevel)
        {
            const auto& l_gameObject = l_gameObjectWeak.lock();

            if (!l_gameObject ||
                l_gameObject->GetVALIsDestroyed())
            {
                continue;
            }

            l_gameObject->Update();
        }
    }
}
void FWK::Scene::LateUpdate() const
{
    for (const auto& l_gameObjectExecutionLevel : m_gameObjectExecutionLevelList)
    {
        for (const auto& l_gameObjectWeak : l_gameObjectExecutionLevel)
        {
            const auto& l_gameObject = l_gameObjectWeak.lock();

            if (!l_gameObject ||
                l_gameObject->GetVALIsDestroyed())
            {
                continue;
            }

            l_gameObject->LateUpdate();
        }
    }
}
void FWK::Scene::PostLateUpdate() const
{
    for (const auto& l_gameObjectExecutionLevel : m_gameObjectExecutionLevelList)
    {
        for (const auto& l_gameObjectWeak : l_gameObjectExecutionLevel)
        {
            const auto& l_gameObject = l_gameObjectWeak.lock();

            if (!l_gameObject ||
                l_gameObject->GetVALIsDestroyed())
            {
                continue;
            }

            l_gameObject->PostLateUpdate();
        }
    }
}

nlohmann::json FWK::Scene::Serialize()
{
    return m_jsonConverter.Serialize(*this);
}

void FWK::Scene::AddGameObject(const std::shared_ptr<GameObject>& a_gameObject)
{
    if (!a_gameObject)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "GameObjectクラスが無効となっており、ゲームオブジェクトの追加処理に失敗しました。");
        return;
    }

    
    // 親GameObjectは、子GameObjectより先に
    // Sceneへ登録されなければならない
    const auto& l_executionLevel = CalculateGameObjectExecutionLevel(a_gameObject);

    // GameObject自身を識別するSceneInstanceUUIDをUUIDRegistryへ登録する
    // 新規GameObjectでUUIDがnilの場合はUUIDRegistry内で新規発行する
    // Deserialize済みで既にUUIDを持っている場合は、そのUUIDを維持したまま
    // Registry内で重複していないことを確認して登録する
    FWK_ASSERT_RETURN_IF(!m_gameObjectUUIDRegistry.Add(a_gameObject, a_gameObject->GetREFSceneInstanceUUID()),"GameObjectのSceneInstanceUUID登録に失敗したため、GameObjectをSceneへ追加できませんでした。");

    m_gameObjectList.emplace_back(a_gameObject);

    // 計算済みの階層へ直接追加する
    AddGameObjectToExecutionLevelList(a_gameObject, l_executionLevel);
}

std::filesystem::path FWK::Scene::FetchVALNextLoadSceneFilePath() const
{
    // 次のに移行するシーンの名前が空なら移行しない
    if (m_nextSceneUUID.is_nil()) { return {}; }

    const auto* l_assetFilePathData = m_assetFilePathRegistry.FindPTRAssetFilePathData(m_nextSceneUUID);

    if (!l_assetFilePathData)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "次のシーンへの遷移用のUUIDが無効です、SceneManagerのマップ内部を確認してください。");

        return {};
    }

    // 次のシーンのファイルパスが空なら移行しない
    if (l_assetFilePathData->m_assetFilePath.empty() ||
        l_assetFilePathData->m_type != Enum::AssetFilePathRegistryType::Scene)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "次のシーンへの情報が無効です、SceneManagerのマップ内部を確認してください。");

        return {};
    }

    // AssetFilePathDataから次に読み込むファイルのパスをreturn
    return l_assetFilePathData->m_assetFilePath;
}

void FWK::Scene::AddGameObjectToExecutionLevelList(const std::weak_ptr<GameObject>& a_gameObject, const std::size_t& a_executionLevel)
{
    if (a_gameObject.expired()) { return; }

    // 指定された実行階層までリストを追加
    while (m_gameObjectExecutionLevelList.size() <= a_executionLevel)
    {
        m_gameObjectExecutionLevelList.emplace_back();
    }

    m_gameObjectExecutionLevelList[a_executionLevel].emplace_back(a_gameObject);
}

void FWK::Scene::RemoveDestroyedGameObjects()
{
    // Scene所有リストから削除
    std::erase_if(m_gameObjectList, 
                 [this](const auto& a_gameObject)
                 {
                     if (!a_gameObject) { return true; }
                 
                     a_gameObject->GetREFSceneInstanceUUID();

                     if (a_gameObject->GetVALIsDestroyed())
                     {
                         FWK_ASSERT_RETURN_VALUE_IF(!m_gameObjectUUIDRegistry.Erase(a_gameObject->GetREFSceneInstanceUUID()), "削除対象GameObjectのUUID登録解除に失敗しました。", false);
                     }
                 
                     return a_gameObject->GetVALIsDestroyed();
                 });

    // 階層実行リストから削除
    for (auto& l_gameObjectExecutionLevel : m_gameObjectExecutionLevelList)
    {
        std::erase_if(l_gameObjectExecutionLevel, 
                      [](const auto& a_gameObjectWeak)
                      {
                          const auto& l_gameObject = a_gameObjectWeak.lock();
                      
                          // weak_ptrの参照先がなくなっている場合も実行リストから削除する
                          return !l_gameObject;
                      });
    }

    // 最後尾が空にならずそれより上の階層が空になる現象などありえないため
    // pop_backを使用する。
    // 後方に残った空階層を削除
    // 階層が0,1,2,3とあった時に急に2を含む要素が
    // 消えてしまっても問題がないようにpop_backで後ろから削除
    while (!m_gameObjectExecutionLevelList.empty() &&
           m_gameObjectExecutionLevelList.back().empty())
    {
        m_gameObjectExecutionLevelList.pop_back();
    }
}

void FWK::Scene::RebuildGameObjectExecutionLevelList()
{
    m_gameObjectExecutionLevelList.clear();

    // UUIDRegistryには既に登録済みなので、
    // m_gameObjectList内の順番にGameObjectの現在改装を求める
    for (const auto& l_gameObject : m_gameObjectList)
    {
        if (!l_gameObject ||
            l_gameObject->GetVALIsDestroyed())
        {
            continue;
        }

        const auto& l_executionLevel = CalculateGameObjectExecutionLevel(l_gameObject);

        // 階層を調べてから適した階層に追加
        AddGameObjectToExecutionLevelList(l_gameObject, l_executionLevel);
    }
}

std::size_t FWK::Scene::CalculateGameObjectExecutionLevel(const std::weak_ptr<GameObject>& a_gameObject) const
{
    const auto& l_gameObject = a_gameObject.lock();

    if (!l_gameObject) { return k_initialExecutionLevel; }

    // TODO
    return k_initialExecutionLevel;
}