#include "InputComponentJsonConverter.h"

void FWK::Converter::InputComponentJsonConverter::DeserializePrefab(const nlohmann::json& a_rootJson, InputComponent& a_inputComponent) const
{
	if (a_rootJson.is_null()) { return; }

	if (const auto& l_json = a_rootJson.value(k_inspectorJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_inspector = a_inputComponent.GetMutableREFInspector();

		l_inspector.Deserialize(l_json);
	}

	if (const auto& l_json = a_rootJson.value(k_notifyComponentEventExecutionConditionListJsonKey, nlohmann::json{});
		!l_json.is_null() &&
		Utility::IsJsonArray(l_json))
	{
		DeserializeNotifyComponentEventExecutionCondition(l_json, a_inputComponent);
	}
}

nlohmann::json FWK::Converter::InputComponentJsonConverter::SerializePrefab(const InputComponent& a_inputComponent) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_inspector = a_inputComponent.GetREFInspector();

	l_rootJson[k_inspectorJsonKey]                                  = l_inspector.Serialize                          ();
	l_rootJson[k_notifyComponentEventExecutionConditionListJsonKey] = SerializeNotifyComponentEventExecutionCondition(a_inputComponent);

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