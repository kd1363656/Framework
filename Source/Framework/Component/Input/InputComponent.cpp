#include "InputComponent.h"

void FWK::InputComponent::INIT()
{
	m_inspector.INIT();
}

void FWK::InputComponent::EditInspector()
{
	m_inspector.EditInspector(*this);
}

void FWK::InputComponent::AddExecutionConditionList(const Struct::ObserverInputExecutionCondition<Enum::ComponentEvent>& a_executionCondition)
{
	// 同じ要素を持つデータを格納しない(重複するだけ無駄であるから)
	if (const auto l_itr = std::find_if(m_executionConditionList.begin(), m_executionConditionList.end(), [&a_executionCondition](const auto& l_executionCondition)
		{
			return a_executionCondition.m_receiveComponentEvent == l_executionCondition.m_receiveComponentEvent;
		});
		l_itr != m_executionConditionList.end())
	{
		return; 
	}

	m_executionConditionList.emplace_back(a_executionCondition);
}