#include "InputComponentJsonConverter.h"

void FWK::Converter::InputComponentJsonConverter::DeserializePrefab(const nlohmann::json& a_rootJson, InputComponent& a_inputComponent) const
{
	if (a_rootJson.is_null()) { return; }

	// NodeEditor全体とStartノードを復元する
	if (const auto& l_json = a_rootJson.value(k_inspectorJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_inspector = a_inputComponent.GetMutableREFInspector();

		l_inspector.Deserialize(l_json);
	}

	// ConditionノードとConditionの判定に内容を復元する
	if (const auto& l_json = a_rootJson.value(k_notifyComponentEventExecutionConditionListJsonKey, nlohmann::json{});
		!l_json.is_null() &&
		Utility::IsJsonArray(l_json))
	{
		DeserializeNotifyComponentEventExecutionCondition(l_json, a_inputComponent);
	}

	// Executeノードと通知内容を復元する
	if (const auto& l_json = a_rootJson.value(k_executionJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		DeserializeExecution(l_json, a_inputComponent);
	}

	auto& l_notifyStrategy = a_inputComponent.GetMutableREFNotifyStrategy();

	// 通知戦略クラスの復元
	Utility::DeserializeInstanceType<TypeAlias::ComponentEventNotifyStrategyUniqueFactory>(a_rootJson, k_notifyStrategyJsonKey, l_notifyStrategy);
}

nlohmann::json FWK::Converter::InputComponentJsonConverter::SerializePrefab(const InputComponent& a_inputComponent) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_inspector      = a_inputComponent.GetREFInspector     ();
	const auto& l_notifyStrategy = a_inputComponent.GetREFNotifyStrategy();

	l_rootJson[k_inspectorJsonKey]                                  = l_inspector.Serialize                          ();
	l_rootJson[k_notifyComponentEventExecutionConditionListJsonKey] = SerializeNotifyComponentEventExecutionCondition(a_inputComponent);
	l_rootJson[k_executionJsonKey]                                  = SerializeExecution                             (a_inputComponent);

	Utility::UpdateJson(l_rootJson, Utility::SerializeInstanceType(l_notifyStrategy, k_notifyStrategyJsonKey));

	return l_rootJson;
}

void FWK::Converter::InputComponentJsonConverter::DeserializeNotifyComponentEventExecutionCondition(const nlohmann::json& a_rootJson, InputComponent& a_inputComponent) const
{ 
	if (a_rootJson.is_null() ||
		!Utility::IsJsonArray(a_rootJson))
	{
		return;
	}

	for (const auto& l_json : a_rootJson)
	{
		if (l_json.is_null()) { continue; }

		Struct::ObserverInputExecutionCondition<Enum::ComponentEvent> l_condition = {};

		l_condition.m_receiveComponentEvent = l_json.value(k_receiveComponentEventJsonKey, Enum::ComponentEvent::Invalid);

		if (const auto& l_nodeEditorJson = l_json.value(k_nodeEditorNodeEventJsonKey, nlohmann::json{});
			!l_nodeEditorJson.is_null())
		{
			l_condition.m_editorNodeEditor.Deserialize(l_nodeEditorJson);
		}

		l_condition.m_checkEventLane         = l_json.value(k_checkEventLaneJsonKey,         Enum::EventLane::Invalid);
		l_condition.m_expectedObserverResult = l_json.value(k_expectedObserverResultJsonKey, false);

		a_inputComponent.AddExecutionConditionList(l_condition);
	}
}
void FWK::Converter::InputComponentJsonConverter::DeserializeExecution(const nlohmann::json& a_rootJson, InputComponent& a_inputComponent) const
{
	if (a_rootJson.is_null()) { return; }

	auto& l_execution = a_inputComponent.GetMutableREFExecution();

	// ExecuteノードがComponentEventObserverへ通知する
	// ComponentEventを復元する
	l_execution.m_notifyComponentEvent = a_rootJson.value(k_notifyComponentEventJsonKey, Enum::ComponentEvent::Invalid);

	// 通知するComponentEventのどのEventLaneか
	l_execution.m_notifyEventLane = a_rootJson.value(k_notifyEventLaneJsonKey, Enum::EventLane::Invalid);

	// 通知する際のフラグがtrueかfalseか
	l_execution.m_notifyFlag = a_rootJson.value(k_notifyFlagJsonKey, false);
}

nlohmann::json FWK::Converter::InputComponentJsonConverter::SerializeNotifyComponentEventExecutionCondition(const InputComponent& a_inputComponent) const
{
	      auto  l_rootJsonArray                              = nlohmann::json::array                                            ();
	const auto& l_notifyComponentEventExecutionConditionList = a_inputComponent.GetREFNotifyComponentEventExecutionConditionList();

	for (const auto& l_condition : l_notifyComponentEventExecutionConditionList)
	{
		nlohmann::json l_json = {};

		l_json[k_receiveComponentEventJsonKey]  = l_condition.m_receiveComponentEvent;
		l_json[k_nodeEditorNodeEventJsonKey]    = l_condition.m_editorNodeEditor.Serialize();
		l_json[k_checkEventLaneJsonKey]         = l_condition.m_checkEventLane;
		l_json[k_expectedObserverResultJsonKey] = l_condition.m_expectedObserverResult;

		l_rootJsonArray.emplace_back(l_json);
	}

	return l_rootJsonArray;
}
nlohmann::json FWK::Converter::InputComponentJsonConverter::SerializeExecution(const InputComponent& a_inputComponent) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_execution = a_inputComponent.GetREFExecution();
	   
	l_rootJson[k_notifyComponentEventJsonKey] = l_execution.m_notifyComponentEvent;
	l_rootJson[k_notifyEventLaneJsonKey]      = l_execution.m_notifyEventLane;
	l_rootJson[k_notifyFlagJsonKey]           = l_execution.m_notifyFlag;
	
	return l_rootJson;
}