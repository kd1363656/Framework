#include "GameObject.h"

void FWK::GameObject::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}
void FWK::GameObject::DeserializePrefab(const nlohmann::json& a_rootJson, std::vector<std::shared_ptr<ComponentBase>>& a_componentList, std::vector<Struct::ChildDeserializeData>& a_childDeserializeData)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.DeserializePrefab(a_rootJson, a_componentList, a_childDeserializeData);
}
void FWK::GameObject::DeserializeScene(const nlohmann::json & a_rootJson, std::vector<std::shared_ptr<ComponentBase>>&a_componentList, std::vector<Struct::ChildDeserializeData>&a_childDeserializeData)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.DeserializeScene(a_rootJson, a_componentList, a_childDeserializeData);
}

void FWK::GameObject::PostDeserialize() const
{
	for (const auto& l_componentData : m_componentSmartPointerVectorArray.GetREFArrayElementDataList())
	{
		const auto& l_component = l_componentData.m_type;

		if (!l_component) { return; }

		l_component->PostDeserialize();
	}
}

void FWK::GameObject::EarlyUpdate() const
{
	for (const auto& l_componentData : m_componentSmartPointerVectorArray.GetREFArrayElementDataList())
	{
		const auto& l_component = l_componentData.m_type;

		if (!l_component) { return; }

		l_component->EarlyUpdate();
	}
}
void FWK::GameObject::Update() const
{
	for (const auto& l_componentData : m_componentSmartPointerVectorArray.GetREFArrayElementDataList())
	{
		const auto& l_component = l_componentData.m_type;

		if (!l_component) { return; }

		l_component->Update();
	}
}
void FWK::GameObject::LateUpdate() const
{
	for (const auto& l_componentData : m_componentSmartPointerVectorArray.GetREFArrayElementDataList())
	{
		const auto& l_component = l_componentData.m_type;

		if (!l_component) { return; }

		l_component->LateUpdate();
	}
}
void FWK::GameObject::ConfirmMatrix() const
{
	FWK_ASSERT_RETURN_IF(!m_transformComponent, "TransformComponentが存在しません、TransformComponentは必ず存在するべきComponentです。");

	m_transformComponent->ConfrimMatrix();
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

void FWK::GameObject::EditInsepector()
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
		FWK_ADD_LOG("GameObject : {}\nコンポーネントが無効となっており割り当てに失敗しました。", m_selfName);

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

void FWK::GameObject::ApplyParent(const std::weak_ptr<GameObject>& a_child)
{
	const auto& l_child = a_child.lock();

	// 自分自身を子にしない
	if (l_child.get() == this) { return; }

	// このゲームオブジェクトを親としてセットし親子関係を構築
	l_child->SetParent(weak_from_this());

	m_childSmartPointerVectorArray.Add(l_child);

	auto l_childTransformComponent = l_child->GetVALTransformComponent().lock();

	FWK_ASSERT_RETURN_IF(!l_childTransformComponent, "子にTransformComponentが存在しません。TransformComponentは絶対にゲームオブジェクトにあるべきものです。");

	// 子のTransformComponentに親のTransformComponentのポインタを渡し
	// 親に追従する用に行列計算方式を置き換える
	l_childTransformComponent->ApplyParentTransformComponent(m_transformComponent);
}

void FWK::GameObject::Unparent(const std::weak_ptr<FWK::GameObject>&a_child)
{
	const auto& l_child = a_child.lock();

	if (!l_child) { return; }

	m_childSmartPointerVectorArray.RemoveSameElement(a_child);
}