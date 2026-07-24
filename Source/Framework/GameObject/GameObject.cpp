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

	m_componentList.Add(a_component);
}