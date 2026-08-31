#include "Scene.h"
#include "../../Application/Application.h"

void FWK::Scene::INIT()
{
	m_gameObjectList.clear              ();
	m_gameObjectExecutionLevelList.clear();

	m_gameObjectUUIDRegistry.Clear();

	m_prefabSystem.INIT               ();
	m_lightSystem.ApplyDefaultSettings();

	m_sceneName.clear();

	// 次にGameObjectを追加した後で、
    // 階層別実行順を再構築できるようにする。
	m_isGameObjectExecutionLevelListDirty = false;
}

void FWK::Scene::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) 
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "Jsonの中身が空となっておりシーンのデシリアライズ処理に失敗しました。");
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
	// 前フレームまでに削除要求されたGameObjectを
	// Scene所有リストから一括削除する
	RemoveDestroyedGameObjects();

	// GameObjectの追加、削除、親子変更があった場合だけ、
	// 階層別実行順を再構築する
	RefreshGameObjectExecutionLevelListIfNeeded();

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
		FWK_ADD_LOG(Constant::k_debugWarningColor, "GameObjectクラスが無効となっており、ゲームオブジェクトの追加処理に失敗しました。");
		return;
	}

	std::size_t l_executionLevel = k_initialExecutionLevel;

	// 親GameObjectは、子GameObjectより先に
	// Sceneへ登録されなければならない
	CalculateGameObjectExecutionLevel(a_gameObject, l_executionLevel);

	// GameObject自身を識別するSceneInstanceUUIDをUUIDRegistryへ登録する
    // 新規GameObjectでUUIDがnilの場合はUUIDRegistry内で新規発行する
    // Deserialize済みで既にUUIDを持っている場合は、そのUUIDを維持したまま
    // Registry内で重複していないことを確認して登録する
    FWK_ASSERT_RETURN_IF(!m_gameObjectUUIDRegistry.Add(a_gameObject, a_gameObject->GetMutableREFSceneInstanceUUID()),"GameObjectのSceneInstanceUUID登録に失敗したため、GameObjectをSceneへ追加できませんでした。");
	
	m_gameObjectList.emplace_back(a_gameObject);

	// Prefabの代表GameObjectが削除などで空になっている場合だけ
	// 今回追加したGameObjectを新しい代表として設定する
	m_prefabSystem.CachePrefabGameObjectIfNeeded(a_gameObject);

	// 計算済みの階層へ直接追加する
	AddGameObjectToExecutionLevelList(a_gameObject, l_executionLevel);
}

std::weak_ptr<FWK::GameObject> FWK::Scene::FindVALGameObject(const boost::uuids::uuid& a_uuid) const
{
	return m_gameObjectUUIDRegistry.FindVALRegisteredType(a_uuid);
}

void FWK::Scene::RemoveDestroyedGameObjects()
{
	bool l_hasRemoveTarget = false;

	// Scene所有を解除する前にUUID登録を解除する
	for (const auto& l_gameObject : m_gameObjectList)
	{
		if (!l_gameObject)
		{
			l_hasRemoveTarget = true;

			continue;
		}

		if (!l_gameObject->GetVALIsDestroyed()) { continue; }

		const auto& l_prefabUUID             = l_gameObject->GetREFPrefabUUID            ();
		const auto  l_prefabSceneInstanceNUM = l_gameObject->GetVALPrefabSceneInstanceNUM();

		// 有効なPrefabUUIDとInstanceNUMを持っている場合だけ、
		// PrefabSystemへInstanceNUMを返却する
		if (!l_prefabUUID.is_nil() &&
			l_prefabSceneInstanceNUM != Constant::k_invalidPrefabSceneInstanceNUM)
		{
			m_prefabSystem.ReleasePrefabInstanceNUM(l_prefabUUID, l_prefabSceneInstanceNUM);

			// UUID登録解除などが失敗して次のフレームに残った場合に、
			// 同じ番号を二重解放しないようにGameObject側を無効値へ戻す
			l_gameObject->SetPrefabSceneInstanceNUM(Constant::k_invalidPrefabSceneInstanceNUM);
		}

		FWK_ASSERT_RETURN_IF(!m_gameObjectUUIDRegistry.Erase(l_gameObject->GetMutableREFSceneInstanceUUID()), "削除対象GameObjectのUUID登録解除に失敗しました。");

		l_hasRemoveTarget = true;
	}

	if (!l_hasRemoveTarget) { return; }

	// 階層実行リストから削除
	for (auto& l_gameObjectExecutionLevel : m_gameObjectExecutionLevelList)
	{
		std::erase_if(l_gameObjectExecutionLevel, [](const auto& a_gameObjectWeak)
		{
			const auto& l_gameObject = a_gameObjectWeak.lock();

			// weak_ptrの参照先がなくなっている場合も実行リストから削除する
			if (!l_gameObject) { return true; }

			return l_gameObject->GetVALIsDestroyed();
		});
	}

	// Scene所有リストから削除
	std::erase_if(m_gameObjectList, [](const auto& a_gameObject)
	{
		if (!a_gameObject) { return true; }

		return a_gameObject->GetVALIsDestroyed();
	});

	// 後方に残った空階層を削除
	// 階層が0,1,2,3とあった時に急に2を含む要素が
	// 消えてしまっても問題がないようにpop_backで後ろから削除
	while (!m_gameObjectExecutionLevelList.empty() &&
		   m_gameObjectExecutionLevelList.back().empty())
	{
		m_gameObjectExecutionLevelList.pop_back();
	}
}

void FWK::Scene::RefreshGameObjectExecutionLevelListIfNeeded()
{
	if (!m_isGameObjectExecutionLevelListDirty) { return; }

	RebuildGameObjectExecutionLevelList();

	m_isGameObjectExecutionLevelListDirty = false;
}

void FWK::Scene::RebuildGameObjectExecutionLevelList()
{
	m_gameObjectExecutionLevelList.clear();

	// UUIDRegistryには既に登録済みなので、
	// m_gameObjectList内の順番にGameObjectの現在改装を求める
	for (const auto& l_gameObject: m_gameObjectList)
	{
		if (!l_gameObject ||
			l_gameObject->GetVALIsDestroyed())
		{
			continue;
		}

		std::size_t l_executionLevel = k_initialExecutionLevel;

		// 階層を調べてから適した階層に追加
		CalculateGameObjectExecutionLevel(l_gameObject, l_executionLevel);
		AddGameObjectToExecutionLevelList(l_gameObject, l_executionLevel);
	}
}

void FWK::Scene::CalculateGameObjectExecutionLevel(const std::weak_ptr<GameObject>& a_gameObject, std::size_t& a_executionLevel) const
{
	const auto& l_gameObject = a_gameObject.lock();

	if (!l_gameObject) { return; }

	auto l_parentGameObject = l_gameObject->GetREFParent().lock();

	while (l_parentGameObject)
	{
		if (l_parentGameObject->GetVALIsDestroyed())
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "削除予定の親GameObjectを持つGameObjectはSceneへ追加できません。");

			return;
		}

		++a_executionLevel;

		l_parentGameObject = l_parentGameObject->GetREFParent().lock();
	}
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