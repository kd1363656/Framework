#include "GameObject.h"

void FWK::GameObject::Deserialize(const nlohmann::json& a_rootJson, Scene& a_scene)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(shared_from_this(), a_rootJson, a_scene);
}
void FWK::GameObject::DeserializePrefab(const nlohmann::json& a_rootJson, Scene& a_scene)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.DeserializePrefab(weak_from_this(), a_rootJson, a_scene);
}
void FWK::GameObject::DeserializePrefab(const nlohmann::json&                                                   a_rootJson, 
	                                          std::vector<Struct::ChildDeserializeData>&                        a_childDeserializeDataList,
	                                          Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray,
	                                          Scene&                                                            a_scene)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.DeserializePrefab(a_rootJson, 
		                              weak_from_this(),
		                              a_childDeserializeDataList,
		                              a_componentSmartPointerVectorArray, 
		                              a_scene);
}
void FWK::GameObject::DeserializeScene(const nlohmann::json &                                                  a_rootJson, 
	                                         std::vector<Struct::ChildDeserializeData>&                        a_childDeserializeDataList,
	                                         Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray,
	                                         Scene&                                                            a_scene)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.DeserializeScene(a_rootJson,
		                             *this,
		                             a_childDeserializeDataList, 
		                             a_componentSmartPointerVectorArray, 
		                             a_scene);
}

void FWK::GameObject::PostDeserialize()
{
	if (m_transformComponent)
	{
		m_transformComponent->SetOwner       (weak_from_this());
		m_transformComponent->PostDeserialize();
	}

	for (const auto& l_componentData : m_componentSmartPointerVectorArray.GetREFArrayElementDataList())
	{
		const auto& l_component = l_componentData.m_type;

		if (!l_component) { continue; }

		l_component->SetOwner       (weak_from_this());
		l_component->PostDeserialize();
	}
}

void FWK::GameObject::EarlyUpdate() const
{
	for (const auto& l_componentData : m_componentSmartPointerVectorArray.GetREFArrayElementDataList())
	{
		const auto& l_component = l_componentData.m_type;

		if (!l_component) { continue; }

		l_component->EarlyUpdate();
	}
}
void FWK::GameObject::Update() const
{
	for (const auto& l_componentData : m_componentSmartPointerVectorArray.GetREFArrayElementDataList())
	{
		const auto& l_component = l_componentData.m_type;

		if (!l_component) { continue; }

		l_component->Update();
	}
}
void FWK::GameObject::LateUpdate() const
{
	for (const auto& l_componentData : m_componentSmartPointerVectorArray.GetREFArrayElementDataList())
	{
		const auto& l_component = l_componentData.m_type;

		if (!l_component) { continue; }

		l_component->LateUpdate();
	}
}
void FWK::GameObject::PostLateUpdate() const
{
	FWK_ASSERT_RETURN_IF(!m_transformComponent, "TransformComponentが存在しません、TransformComponentは必ず存在するべきComponentです。");

	// 当たり判定などで微調整された行列を確定
	m_transformComponent->PostLateUpdate();

	// 行列確定後に決まる処理を更新する
	for (const auto& l_componentData : m_componentSmartPointerVectorArray.GetREFArrayElementDataList())
	{
		const auto& l_component = l_componentData.m_type;

		if (!l_component) { continue; }

		l_component->PostLateUpdate();
	}
}

void FWK::GameObject::Destroy()
{
	m_isDestroyed = true;

	// 子も削除フラグを立てる
	// もし親が削除されて連動して消されたくないような局面が出てきたら
	// その時に処理を書き換えるようにすること、基本は親と連動して削除フラグを立てる
	for (const auto& l_childData : m_childSmartPointerVectorArray.GetMutableREFArrayElementDataList())
	{
		auto l_child = l_childData.m_type.lock();

		if (!l_child) { continue; }

		l_child->Destroy();
	}
}

void FWK::GameObject::EditInspector()
{
	FWK_ASSERT_RETURN_IF(!m_transformComponent, "TransformComponentが存在しません、TrnsformComponentは必ず存在するべきComponentです。");

	m_transformComponent->EditInspector();

	for (const auto& l_compoentData : m_componentSmartPointerVectorArray.GetREFArrayElementDataList())
	{
		const auto& l_compoent = l_compoentData.m_type;

		if (!l_compoent) { continue; }

		l_compoent->EditInspector();
	}
}

nlohmann::json FWK::GameObject::SerializeScene() const
{
	return m_jsonConverter.SerializeScene(*this);
}
nlohmann::json FWK::GameObject::SerializePrefab() const
{
	return m_jsonConverter.SerializePrefab(*this);
}

void FWK::GameObject::AddComponent(const std::shared_ptr<ComponentBase>& a_component)
{
	if (!a_component) 
	{
		FWK_ADD_LOG("GameObject : {}\nコンポーネントが無効となっており割り当てに失敗しました。", FetchVALGameObjectName());

		return; 
	}

	// 派生クラスの静的IDを取得(このコンポーネントを取得時に使用)
	const auto l_staticTypeID = a_component->GetREFRuntimeTypeINFO().k_staticTypeID;

	bool l_canAdd = false;

	// 複数持てるコンポーネントかどうかを判断して
	// 適切なstd::unordered_mapに割り当てる
	if (!a_component->IsAllowMultiple())
	{
		l_canAdd = m_uniqueComponentMap.try_emplace(l_staticTypeID, a_component).second;
	}
	else
	{
		m_multiComponentMap[l_staticTypeID].emplace_back(a_component);

		l_canAdd = true;
	}

	if (!l_canAdd)
	{
		FWK_ADD_LOG("Component : {}\nコンポーネントの格納に失敗しました。", a_component->GetREFTypeINFO().k_name);

		return;
	}

	m_componentSmartPointerVectorArray.Add(a_component);
}
void FWK::GameObject::RemoveComponent(const std::weak_ptr<ComponentBase>& a_component)
{
	const auto& l_component = a_component.lock();

	if (!l_component)
	{
		FWK_ADD_LOG("削除対象Componentが無効のため、GameObjectから削除することができませんでした。");

		return;
	}

	const auto l_staticTypeID = l_component->GetREFRuntimeTypeINFO().k_staticTypeID;

	bool l_isRemovedFromTypeMap = false;

	if (!l_component->IsAllowMultiple())
	{
		auto l_itr = m_uniqueComponentMap.find(l_staticTypeID);

		// イテレータらから探索して、もしなければreturn
		if (l_itr == m_uniqueComponentMap.end()) { return; }

		auto l_registeredComponent = l_itr->second.lock();

		if (!l_registeredComponent)
		{
			m_uniqueComponentMap.erase(l_itr);

			return;
		}

		// 同一アドレスでない場合return
		if (l_registeredComponent != l_component) { return; }

		m_uniqueComponentMap.erase(l_itr);

		l_isRemovedFromTypeMap = true;
	}
	else
	{
		auto l_componentListITR = m_multiComponentMap.find(l_staticTypeID);

		if (l_componentListITR == m_multiComponentMap.end()) { return; }

		// コンポーネントリストをMapから取得
		auto& l_componentList = l_componentListITR->second;
		auto  l_componentITR  = l_componentList.begin();

		// リストから同じアドレスのコンポーネントを見つけ削除する
		while (l_componentITR != l_componentList.end())
		{
			auto l_registeredComponent = l_componentITR->lock();

			if (!l_registeredComponent)
			{
				l_componentITR = l_componentList.erase(l_componentITR);

				continue;
			}

			if (l_registeredComponent != l_component)
			{
				++l_componentITR;

				continue;
			}

			l_componentList.erase(l_componentITR);

			l_isRemovedFromTypeMap = true;

			break;
		}

		if (l_componentList.empty())
		{
			m_multiComponentMap.erase(l_componentListITR);
		}
	}

	if (!l_isRemovedFromTypeMap) { return; }

	m_componentSmartPointerVectorArray.RemoveSameElement(l_component);
}

void FWK::GameObject::ApplyParent(const std::weak_ptr<GameObject>& a_child)
{
	const auto& l_child = a_child.lock();

	if (!l_child) { return; }
	
	const auto& l_self = shared_from_this();

	if (!l_self) { return; }

	// 自分自身を子にはできない
	if (l_child == l_self) { return; }

	// 循環防止
	if (IsDescendantOf(l_child))
	{
		FWK_ADD_LOG("GameObjectの親子関係が循環するため、親子関係を構築できませんでした。");

		return;
	}

	const auto& l_childTransformComponent = l_child->GetVALTransformComponent().lock();

	FWK_ASSERT_RETURN_IF(!l_childTransformComponent, "子GameObjectにTransformComponentが存在しません。");

	// すでに同じ親の場合return
	const auto l_currentParent = l_child->GetREFParent().lock();

	if (l_currentParent == l_self) { return; }

	// 以前の親を解除
	if (l_currentParent)
	{
		l_currentParent->Unparent(l_child);
	}

	// GameObject親子関係の構築
	l_child->SetParent(l_self);

	const std::weak_ptr<GameObject> l_childWeak = l_child;

	m_childSmartPointerVectorArray.Add(l_childWeak);

	l_childTransformComponent->ApplyParent(l_self);
}

void FWK::GameObject::Unparent(const std::weak_ptr<FWK::GameObject>&a_child)
{
	const auto& l_child = a_child.lock();

	if (!l_child) { return; }

	// 自分が親ではないGameObjectは解除しない
	if (const auto& l_currentParent = l_child->GetREFParent().lock();
		l_currentParent != shared_from_this())
	{
		return; 
	}

	const auto l_childTransformComponent = l_child->GetVALTransformComponent().lock();

	FWK_ASSERT_RETURN_IF(!l_childTransformComponent, "子GameObjectにTransformComponentが存在しないため、親子関係を解除できませんでした。");

	// 子の情報を削除し、子から親の情報を削除する
	m_childSmartPointerVectorArray.RemoveSameElement(a_child);
	l_child->GetMutableREFParent                    ().reset();

	// 親がいなくても行列を計算できるようにする
	l_childTransformComponent->ApplyStandalone();
}

std::string FWK::GameObject::FetchVALGameObjectName() const
{
	// FBXをSceneへ置いただけで、
	// まだPrefab化されていないGAmeObjectは
	// Prefab名だけを表示する
	if (m_prefabInstanceNUM == Constant::k_invalidPrefabInstanceNUM)
	{
		if (!m_prefabName.empty()) { return m_prefabName; }

		return std::string{ Constant::k_gameObjectString };
	}

	FWK_ASSERT_RETURN_VALUE_IF(m_prefabName.empty(), "PrefabInstanceNUMが有効なのにPrefabNameが空になっています。", std::string{ Constant::k_gameObjectString });
	
	return std::format("{}_{}", m_prefabName, m_prefabInstanceNUM);
}

bool FWK::GameObject::IsDescendantOf(const std::shared_ptr<GameObject>& a_ancestor) const
{
	if (!a_ancestor) { return false; }

	auto l_parent = m_parent.lock();

	// 自身から親方向へさかのぼって
	// してGameObjectが祖先に存在するか確認する
	while (l_parent)
	{
		if (l_parent == a_ancestor) { return true; }

		l_parent = l_parent->GetREFParent().lock();
	}

	return false;
}