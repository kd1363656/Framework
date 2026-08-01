#include "Scene.h"
#include "../../Application/Application.h"

void FWK::Scene::INIT()
{
	m_gameObjectList.clear        ();
	m_gameObjectUUIDRegistry.Clear();

	m_prefabSystem.INIT               ();
	m_lightSystem.ApplyDefaultSettings();
}
void FWK::Scene::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) 
	{
		FWK_ADD_LOG("jsonの中身が空となっておりシーンのデシリアライズ処理に失敗しました。");
		return; 
	}

	m_jsonConverter.Deserialize(a_rootJson, *this);
}
void FWK::Scene::PostDeserialize() const
{
	for (const auto& l_gameObject : m_gameObjectList)
	{
		if (!l_gameObject) { continue; }

		l_gameObject->PostDeserialize();
	}

	auto& l_physicsManager = Physics::PhysicsManager::GetInstance();

	// StaticBodyをまとめて追加した後なので、BroadPhaseを最適化しておく。
	// 毎フレーム呼ぶものではなく、ステージ読み込み後などに呼ぶ。
	l_physicsManager.OptimizeBroadPhase();
}

void FWK::Scene::EarlyUpdate()
{
	std::erase_if(m_gameObjectList, [this](const auto& a_gameObject) 
	{
		if (!a_gameObject) { return true; }

		// 削除予定ならGUIDRegistryからも削除
		if (a_gameObject->GetVALIsDestroyed())
		{
			m_gameObjectUUIDRegistry.Erase(a_gameObject->GetMutableREFUUID());

			return true;
		}
		
		return false;
	});

	for (const auto& l_gameObject : m_gameObjectList)
	{
		if (!l_gameObject) { continue; }

		l_gameObject->EarlyUpdate();
	}
}
void FWK::Scene::Update()
{
	for (const auto& l_gameObject : m_gameObjectList)
	{
		if (!l_gameObject) { continue; }

		l_gameObject->Update();
	}
}
void FWK::Scene::LateUpdate() const
{
	for (const auto& l_gameObject : m_gameObjectList)
	{
		if (!l_gameObject) { continue; }

		l_gameObject->LateUpdate();
	}
}
void FWK::Scene::ConfirmMatrix() const
{
	for (const auto& l_gameObject : m_gameObjectList)
	{
		if (!l_gameObject) { continue; }

		l_gameObject->ConfirmMatrix();
	}
}

nlohmann::json FWK::Scene::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

void FWK::Scene::AddGameObject(const std::shared_ptr<GameObject>& a_gameObject)
{
	if (!a_gameObject)
	{
		FWK_ADD_LOG("GameObjectクラスが無効となっており、ゲームオブジェクトの追加処理に失敗しました。");
		return;
	}

	// ゲームオブジェクトとそのUUIDを登録、ただしUUIDがGUID_NULLだったり、
	// 重複するUUIDの場合UUIDを生成してゲームオブジェクト側のUUIDにも反映する
	FWK_ASSERT_RETURN_IF(!m_gameObjectUUIDRegistry.Add(a_gameObject, a_gameObject->GetMutableREFUUID()), "ゲームオブジェクトのUUIDの登録に失敗しており、ゲームオブジェクトの追加処理に失敗しました。");

	m_gameObjectList.emplace_back(a_gameObject);
}

std::weak_ptr<FWK::GameObject> FWK::Scene::FindVALGameObject(const UUID& a_uuid) const
{
	return m_gameObjectUUIDRegistry.FindVALRegisteredType(a_uuid);
}