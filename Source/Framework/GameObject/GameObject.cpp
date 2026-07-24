#include "GameObject.h"

void FWK::GameObject::PostDeserialize() const
{
	for (const auto& l_componentData : m_componentList.GetREFArrayElementDataList())
	{
		const auto& l_component = l_componentData.m_type;

		if (!l_component) { return; }

		l_component->PostDeserialize();
	}
}

void FWK::GameObject::EarlyUpdate() const
{
	for (const auto& l_componentData : m_componentList.GetREFArrayElementDataList())
	{
		const auto& l_component = l_componentData.m_type;

		if (!l_component) { return; }

		l_component->EarlyUpdate();
	}
}
void FWK::GameObject::Update() const
{
	for (const auto& l_componentData : m_componentList.GetREFArrayElementDataList())
	{
		const auto& l_component = l_componentData.m_type;

		if (!l_component) { return; }

		l_component->Update();
	}
}
void FWK::GameObject::LateUpdate() const
{
	for (const auto& l_componentData : m_componentList.GetREFArrayElementDataList())
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

	m_componentList.Add(a_component);
}