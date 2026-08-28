#include "InputComponent.h"

void FWK::InputComponent::INIT()
{
	m_inspector.INIT();
}

void FWK::InputComponent::DeserializePrefab(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.DeserializePrefab(a_rootJson, *this);
}

void FWK::InputComponent::EarlyUpdate()
{
	const auto& l_gameObject = GetREFOwner().lock();

	if (!l_gameObject) 
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "InputComponentの所有者であるゲームオブジェクトの取得に失敗しましたゲームオブジェクト");

		return; 
	}

	auto& l_componentEventObserver = l_gameObject->GetREFComponentEventObserver();

	if (!l_componentEventObserver) 
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "コンポーネントイベントオブザーバーが無効となっています。");

		return; 
	}

	// 通知できる状態でなければreturn
	if (!CanNotifyEvent(*l_componentEventObserver)) { return; }

	// Executeノードで通知するComponentEventが設定されていなければ
	// 通知する内容が存在しないため処理を終了する
	if (m_execution.m_notifyComponentEvent == Enum::ComponentEvent::Invalid) { return; }
	if (m_execution.m_notifyEventLane == Enum::EventLane::Invalid)           { return; }
	
	NotifyEvent();
}

nlohmann::json FWK::InputComponent::SerializePrefab()
{
	return m_jsonConverter.SerializePrefab(*this);
}

void FWK::InputComponent::EditInspector()
{
	m_inspector.EditInspector(*this);
}

bool FWK::InputComponent::CanNotifyEvent(Observer<Enum::ComponentEvent>& a_componentEventObserver)
{
	// 全ての該当するイベントレーンからの通知を確認し、期待する結果でない通知が届いていた場合falseを返す
	if (std::ranges::any_of(m_notifyComponentEventExecutionConditionList, 
		                   [&a_componentEventObserver](const auto& a_condition) 
		                   {
		                   		return a_componentEventObserver.IsEventMatching(a_condition.m_receiveComponentEvent, a_condition.m_checkEventLane) != a_condition.m_expectedObserverResult;
		                   }))
	{
		return false;
	}

	return true;
}

void FWK::InputComponent::AddExecutionConditionList(const Struct::ObserverInputExecutionCondition<Enum::ComponentEvent>& a_executionCondition)
{
	// 同じ要素を持つデータを格納しない(重複するだけ無駄であるから)
	if (std::ranges::any_of(m_notifyComponentEventExecutionConditionList,
		                    [&a_executionCondition](const auto& a_listExecutionCondition)
		                    {
		                    	return a_executionCondition.m_receiveComponentEvent == a_listExecutionCondition.m_receiveComponentEvent;
		                    }))
	{
		return; 
	}

	m_notifyComponentEventExecutionConditionList.emplace_back(a_executionCondition);
}

void FWK::InputComponent::NotifyEvent()
{
	if (!m_notifyStrategy) { return; }

	m_notifyStrategy->Execute(*this);
}