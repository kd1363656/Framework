#include "GameObject.h"

void FWK::GameObject::INIT()
{
	m_uniqueComponentMap.clear();
	m_multiComponentMap.clear ();

	if (!m_transformComponent)
	{
		m_transformComponent = std::make_shared<TransformComponent>();
	}

	m_transformComponent->INIT();

	m_parent.reset();

	m_childSmartPointerVectorArray.Clear    ();
	m_componentSmartPointerVectorArray.Clear();

	m_componentEventObserver.INIT();

	m_jsonConverter = {};

	m_prefabUUID        = {};
	m_sceneInstanceUUID = {};
	
	m_prefabName.clear();

	m_prefabInstanceNUM = Constant::k_invalidPrefabInstanceNUM;

	m_isDestroyed = false;
}

void FWK::GameObject::Deserialize(const nlohmann::json& a_rootJson, std::unordered_set<boost::uuids::uuid>& a_parentPrefabUUIDSet, Scene& a_scene)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(shared_from_this(), 
		                        a_rootJson, 
		                        a_parentPrefabUUIDSet,
		                        a_scene);
}
void FWK::GameObject::DeserializePrefab(const nlohmann::json& a_rootJson, Scene& a_scene)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.DeserializePrefab(weak_from_this(), a_rootJson, a_scene);
}
void FWK::GameObject::DeserializePrefab(const nlohmann::json&                                                   a_rootJson, 
	                                          std::vector<Struct::ChildDeserializeData>&                        a_childDeserializeDataList,
	                                          Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray,
	                                          std::unordered_set<boost::uuids::uuid>&                           a_parentPrefabUUIDSet,
	                                          Scene&                                                            a_scene)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.DeserializePrefab(a_rootJson, 
		                              weak_from_this(),
		                              a_childDeserializeDataList,
		                              a_componentSmartPointerVectorArray, 
		                              a_parentPrefabUUIDSet,
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
	FWK_ASSERT_RETURN_IF(!m_transformComponent, "TransformComponentが存在しません、TransformComponentは必ず存在するべきComponentです。");

	m_transformComponent->SetOwner       (weak_from_this());
	m_transformComponent->PostDeserialize();

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

bool FWK::GameObject::ApplyParent(const std::weak_ptr<GameObject>& a_child)
{
	const auto& l_child = a_child.lock();

	if (!l_child) { return false; }
	
	const auto& l_self = shared_from_this();

	if (!l_self) { return false; }

	// 自分自身を子にするとGameObjectの親子関係が即座に循環するため許可しない
	if (l_child == l_self) { return false; }

	if (l_child->GetREFPrefabUUID().is_nil()) { return false; }

	// 親子構築するGameObjectは、
	// PrefabUUIDとPrefabInstanceNUMの両方を持つ
	// 有効なPrefabInstanceである必要がある
	if (!Utility::IsPrefabInstance(*l_self) ||
		!Utility::IsPrefabInstance(*l_child))
	{
		FWK_ADD_LOG("PrefabInstanceではないGameObject同士では親子関係を構築できませんでした。");

		return false;
	}

	// 自分の祖先にしようとしていてるGameObjectを子へ設定すると
	// GameObjectそのものの親子関係が循環するため拒否する
	if (IsDescendantOf(l_child))
	{
		FWK_ADD_LOG("GameObjectの親子関係が循環するため、親子関係を構築できませんでした。");

		return false;
	}

	const auto& l_childTransformComponent = l_child->GetVALTransformComponent().lock();

	FWK_ASSERT_RETURN_VALUE_IF(!l_childTransformComponent, "子GameObjectにTransformComponentが存在しません。", false);

	std::unordered_set<boost::uuids::uuid> l_prefabUUIDSet = {};
	
	// 新しい親となる自分自身からRootまで辿り
	// 同一PrefabUUIDが尾内親経路に存在しないことを確認する
	auto l_parentGameObject = l_self;

	// まずはRootからPrefabの重複があるかどうかを確認
	while (l_parentGameObject)
	{		
		if (l_parentGameObject->GetREFPrefabUUID().is_nil()) { return false; }

		// Hierarchyへ登録されているGameObjectは、有効なPrefabInstanceでなければならない
		if (!Utility::IsPrefabInstance(*l_parentGameObject))
		{
			FWK_ADD_LOG("親階層にPrefabInstanceではないGameObjectが存在するため、親子関係を構築できませんでした。");

			return false;
		}

		// unordered_set::emplace()の戻り値を利用して、
		// contains()とemplace()を別々に呼ばず
		// 一度の検索で重複確認と登録を行う
		if (const auto& l_prefabUUID = l_parentGameObject->GetREFPrefabUUID();
			!l_prefabUUIDSet.emplace(l_prefabUUID).second)
		{
			FWK_ADD_LOG("親階層に同じPrefabUUIDを持つGameObjectが存在するため、親子関係を構築できませんでした。");

			return false;
		}

		l_parentGameObject = l_parentGameObject->GetREFParent().lock();
	}

	// 親階層だけでなく、
	// 新しく接続する子GameObject以下の階層も含めて
	// 同一路線上に同じPrefabUUIDが存在しないことを確認する
	if (ContainsDuplicatePrefabUUIDRecursive(a_child, l_prefabUUIDSet))
	{
		FWK_ADD_LOG("同じ親子経路上に同じPrefabUUIDが存在するため、親子関係を構築できませんでした。");

		return false;
	}

	const auto& l_currentParent = l_child->GetREFParent().lock();

	// すでに同じGameOBjectが親の場合は二重登録しない
	if (l_currentParent == l_self) { return false; }

	// 現在の親GameObjectが存在する場合は
	// 新しい親を設定する前に親子関係を解除する
	if (l_currentParent)
	{
		l_currentParent->Unparent(l_child);

		// Unpatenr()に失敗して現在の親が残っている場合は
		// 新しい親GameObjectを設定しない
		if (!l_child->GetREFParent().expired())
		{
			FWK_ADD_LOG("以前の親GameObjectとの親子関係を解除できませんでした。");

			return false;
		}
	}

	// GameObject親子関係の構築
	l_child->SetParent(l_self);

	// 親GameObject側へ子GameObjectを登録する
	m_childSmartPointerVectorArray.Add(a_child);

	// TransformComponentへ新しい親GameObjectを適用する。
	l_childTransformComponent->ApplyParent(l_self);

	return true;
}
bool FWK::GameObject::ApplyParent(const std::weak_ptr<GameObject>& a_child, std::unordered_set<boost::uuids::uuid>& a_parentPrefabUUIDSet)
{
	const auto& l_child = a_child.lock();

	if (!l_child                     ||
		l_child->GetVALIsDestroyed() ||
		l_child->GetREFPrefabUUID().is_nil())
	{
		return false;
	}

	// Deserialize対象の子GameObjectも、
	// PrefabUUIDとPrefabInstanceNUMを持つ
	// 有効なPrefabInstanceである必要がある
	if (!Utility::IsPrefabInstance(*l_child))
	{
		FWK_ADD_LOG("子GameObjectがPrefabInstanceではないため、親子関係を構築できませんでした。");

		return false;
	}

	const auto& l_childPrefabUUID = l_child->GetREFPrefabUUID();

	// Root空現在の親までの経路上に
	// 子と同じPrefabUUIDが存在する場合は
	// Prefabの循環参照になるため追加しない
	if (!a_parentPrefabUUIDSet.emplace(l_childPrefabUUID).second)
	{
		FWK_ADD_LOG("親階層と同じPrefabUUIDを持つ子GameObjectは追加できません。");

		return false;
	}

	// GameObject自身の循環確認
	if (!ApplyParent(a_child))
	{
		a_parentPrefabUUIDSet.erase(l_childPrefabUUID);

		return false;
	}

	// 孫以下の処理終了後にConverter側からeraseする
	return true;
}

void FWK::GameObject::Unparent(const std::weak_ptr<GameObject>&a_child)
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

bool FWK::GameObject::ContainsDuplicatePrefabUUIDRecursive(const std::weak_ptr<GameObject>& a_gameObject, std::unordered_set<boost::uuids::uuid>& a_prefabUUIDSet) const
{
	const auto& l_gameObject = a_gameObject.lock();

	if (!l_gameObject                     ||
		l_gameObject->GetVALIsDestroyed() ||
		l_gameObject->GetREFPrefabUUID().is_nil())
	{
		return false;
	}

	bool l_isPrefabUUIDAdded = false;

	// PrefabInstanceとして確定していてGameObjectだけを、
	// `PrefabUUIDの重複確認対象にする
	if (Utility::IsPrefabInstance(*l_gameObject))
	{
		// 現在確認している親子経路上へ
		// 同じPrefabUUIDが既に登録されている場合は
		// Prefabが循環していると判断する
		if (const auto& l_prefabUUID = l_gameObject->GetREFPrefabUUID();
			!a_prefabUUIDSet.emplace(l_prefabUUID).second)
		{
			return true; 
		}

		l_isPrefabUUIDAdded = true;
	}

	const auto& l_childSmartPointerVectorArray = l_gameObject->GetREFChildSmartPointerVectorArray         ();
	const auto& l_childGameObjectDataList      = l_childSmartPointerVectorArray.GetREFArrayElementDataList();

	for (const auto& l_childGameObjectData : l_childGameObjectDataList)
	{
		if (const auto& l_childGameObject = l_childGameObjectData.m_type.lock();
			!l_childGameObject ||
			l_childGameObject->GetVALIsDestroyed())
		{
			continue;
		}

		if (ContainsDuplicatePrefabUUIDRecursive(l_childGameObjectData.m_type, a_prefabUUIDSet))
		{
			// このGameObjecでUUIDをSet追加した場合は、
			// 呼びだし元へ戻る前に登録状態を元へ戻す
			if (l_isPrefabUUIDAdded)
			{
				a_prefabUUIDSet.erase(l_gameObject->GetREFPrefabUUID());
			}

			return true;
		}
	}

	// 兄弟は同じ親子経路ではないため
	// 現在のGameObjectの以下の確認が終了した時点で解除する
	if (l_isPrefabUUIDAdded)
	{
		a_prefabUUIDSet.erase(l_gameObject->GetREFPrefabUUID());
	}

	return false;
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