#include "InputComponentInspectorJsonConverter.h"

void FWK::Converter::InputComponentInspectorJsonConverter::Deserialize(const nlohmann::json& a_rootJson, InputComponentInspector& a_inputComponentInspector) const
{
	if (a_rootJson.is_null()) { return; }

	// NodeEditorAllocatorとLinkDataListを復元する。
	if (const auto& l_json =a_rootJson.value(k_nodeEditorJsonKey, nlohmann::json{}); 
		!l_json.is_null())
	{
		auto& l_nodeEditor = a_inputComponentInspector.GetMutableREFNodeEditor();

		l_nodeEditor.Deserialize(l_json);
	}

	// Startノードの復元
	if (const auto& l_json = a_rootJson.value(k_startNodeEditorNodeJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_startNodeEditorNode = a_inputComponentInspector.GetMutableREFStartNodeEditorNode();

		l_startNodeEditorNode.Deserialize(l_json);
	}

	// コンディションノードの復元
	if (const auto& l_json = a_rootJson.value(k_rootConditionNodeEditorNodeJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_rootConditionNodeEditorNode = a_inputComponentInspector.GetMutableREFRootConditionNodeEditorNode();

		l_rootConditionNodeEditorNode.Deserialize(l_json);
	}

	// Executionノードの復元
	if (const auto& l_json = a_rootJson.value(k_executeNodeEditorNodeJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_executeNodeEditorNode = a_inputComponentInspector.GetMutableREFExecuteNodeEditorNode();

		l_executeNodeEditorNode.Deserialize(l_json);
	}
}

nlohmann::json FWK::Converter::InputComponentInspectorJsonConverter::Serialize(const InputComponentInspector& a_inputComponentInspector) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_nodeEditor                  = a_inputComponentInspector.GetREFNodeEditor                 ();
	const auto& l_startNodeEditorNode         = a_inputComponentInspector.GetREFStartNodeEditorNode        ();
	const auto& l_rootConditionNodeEditorNode = a_inputComponentInspector.GetREFRootConditionNodeEditorNode();
	const auto& l_executeNodeEditorNode       = a_inputComponentInspector.GetREFExecuteNodeEditorNode      ();

	l_rootJson[k_nodeEditorJsonKey]                  = l_nodeEditor.Serialize                 ();
	l_rootJson[k_startNodeEditorNodeJsonKey]         = l_startNodeEditorNode.Serialize        ();
	l_rootJson[k_rootConditionNodeEditorNodeJsonKey] = l_rootConditionNodeEditorNode.Serialize();
	l_rootJson[k_executeNodeEditorNodeJsonKey]       = l_executeNodeEditorNode.Serialize      ();

	return l_rootJson;
}