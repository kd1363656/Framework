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

	if (const auto& l_json = a_rootJson.value(k_startNodeEditorJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_startNodeEditor = a_inputComponentInspector.GetMutableREFStartNodeEditor();

		l_startNodeEditor.Deserialize(l_json);
	}

	// Deserailize下Node座標を次回の描画時に
	// ImNodesへ反映する必要があるためfalseへ戻す
	a_inputComponentInspector.SetIsDefaultNodePositionApplied(false);
}

nlohmann::json FWK::Converter::InputComponentInspectorJsonConverter::Serialize(const InputComponentInspector& a_inputComponentInspector) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_nodeEditor      = a_inputComponentInspector.GetREFNodeEditor     ();
	const auto& l_startNodeEditor = a_inputComponentInspector.GetREFStartNodeEditor();

	l_rootJson[k_nodeEditorJsonKey]      = l_nodeEditor.Serialize     ();
	l_rootJson[k_startNodeEditorJsonKey] = l_startNodeEditor.Serialize();

	return l_rootJson;
}