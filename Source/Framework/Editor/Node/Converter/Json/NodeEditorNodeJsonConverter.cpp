#include "NodeEditorNodeJsonConverter.h"

void FWK::Converter::NodeEditorNodeJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Editor::NodeEditorNode& a_nodeEditorNode) const
{
	if (a_rootJson.is_null()) { return; }

	// InputPinIDListのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_inputPinListJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		DeserializeInputPinIDList(l_json, a_nodeEditorNode);
	}

	// OutputPinIDListのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_outputPinListJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		DeserializeOutputPinIDList(l_json, a_nodeEditorNode);
	}

	a_nodeEditorNode.SetNodeID(a_rootJson.value(k_nodeIDJsonKey, Constant::k_invalidNodeEditorID));
}

nlohmann::json FWK::Converter::NodeEditorNodeJsonConverter::Serialize(const Editor::NodeEditorNode& a_nodeEditorNode) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_inputPinIDList  = a_nodeEditorNode.GetREFInputPinIDList ();
	const auto& l_outputPinIDList = a_nodeEditorNode.GetREFOutputPinIDList();
	const auto  l_nodeID          = a_nodeEditorNode.GetVALNodeID         ();

	l_rootJson[k_inputPinListJsonKey]  = SerializePinIDList(l_inputPinIDList);
	l_rootJson[k_outputPinListJsonKey] = SerializePinIDList(l_outputPinIDList);

	l_rootJson[k_nodeIDJsonKey] = l_nodeID;

	return l_rootJson;
}

void FWK::Converter::NodeEditorNodeJsonConverter::DeserializeInputPinIDList(const nlohmann::json& a_rootJson, Editor::NodeEditorNode& a_nodeEditorNode) const
{
	if (a_rootJson.is_null() ||
		!Utility::IsJsonArray(a_rootJson))
	{
		return;
	}

	for (const auto& l_json : a_rootJson)
	{
		if (l_json.is_null()) { continue; }

		const TypeAlias::NodeEditorID l_pinID = l_json.value(k_pinIDJsonKey, Constant::k_invalidNodeEditorID);

		if (l_pinID == Constant::k_invalidNodeEditorID) { continue; }

		a_nodeEditorNode.AddInputPinID(l_pinID);
	}
}

void FWK::Converter::NodeEditorNodeJsonConverter::DeserializeOutputPinIDList(const nlohmann::json& a_rootJson, Editor::NodeEditorNode& a_nodeEditorNode) const
{
	if (a_rootJson.is_null() ||
		!Utility::IsJsonArray(a_rootJson))
	{
		return;
	}

	for (const auto& l_json : a_rootJson)
	{
		if (l_json.is_null()) { continue; }

		const TypeAlias::NodeEditorID l_pinID = l_json.value(k_pinIDJsonKey, Constant::k_invalidNodeEditorID);

		if (l_pinID == Constant::k_invalidNodeEditorID) { continue; }

		a_nodeEditorNode.AddOutputPinID(l_pinID);
	}
}

nlohmann::json FWK::Converter::NodeEditorNodeJsonConverter::SerializePinIDList(const std::vector<TypeAlias::NodeEditorID>& a_pidIDList) const
{
	auto l_rootJsonArray = nlohmann::json::array();

	for (const auto& l_pinID : a_pidIDList)
	{
		// 無効な値なら処理を飛ばし保存しない
		if (l_pinID == Constant::k_invalidNodeEditorID) { continue; }

		nlohmann::json l_json = {};

		l_json[k_pinIDJsonKey] = l_pinID;

		l_rootJsonArray.emplace_back(l_json);
	}

	return l_rootJsonArray;
}