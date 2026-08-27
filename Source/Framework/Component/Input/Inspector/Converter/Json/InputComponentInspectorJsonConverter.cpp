#include "InputComponentInspectorJsonConverter.h"

void FWK::Converter::InputComponentInspectorJsonConverter::Deserialize(const nlohmann::json& a_rootJson, InputComponentInspector& a_inputComponentInspector) const
{
	if (a_rootJson.is_null()) { return; }

	if (const auto& l_json = a_rootJson.value(k_nodeEditorJsonKey, nlohmann::json{});
	    !l_json.is_null())
	{
		auto& l_nodeEditor = a_inputComponentInspector.GetMutableREFNodeEditor();

		l_nodeEditor.Deserialize(l_json);
	}

	if (const auto& l_json = a_rootJson.value(k_startNodeJsonKey, nlohmann::json{});
	    !l_json.is_null())
	{
		auto& l_startNode = a_inputComponentInspector.GetMutableREFStartNode();

		l_startNode.Deserialize(l_json);
	}
}

nlohmann::json FWK::Converter::InputComponentInspectorJsonConverter::Serialize(const InputComponentInspector& a_inputComponentInspector) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_nodeEditor = a_inputComponentInspector.GetREFNodeEditor();
	const auto& l_startNode  = a_inputComponentInspector.GetREFStartNode ();

	l_rootJson[k_nodeEditorJsonKey] = l_nodeEditor.Serialize();
	l_rootJson[k_startNodeJsonKey]  = l_startNode.Serialize ();

	return l_rootJson;
}