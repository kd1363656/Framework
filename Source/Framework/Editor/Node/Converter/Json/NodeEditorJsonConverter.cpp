#include "NodeEditorJsonConverter.h"

void FWK::Converter::NodeEditorJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Editor::NodeEditor& a_nodeEditor) const
{
	if (a_rootJson.is_null()) { return; }

	if (const auto& l_json = a_rootJson.value(k_nodeEditorAllocatorJsonKey, nlohmann::json{});
	    !l_json.is_null())
	{
		auto& l_nodeEditorAllocator = a_nodeEditor.GetMutableREFNodeEditorAllocator();

		l_nodeEditorAllocator.Deserialize(l_json);
	}

	if (const auto& l_json = a_rootJson.value(k_linkDataListJsonKey, nlohmann::json{});
		!l_json.is_null() &&
		Utility::IsJsonArray(l_json))
	{
		DeserializeLinkDataList(l_json, a_nodeEditor);
	}
}

nlohmann::json FWK::Converter::NodeEditorJsonConverter::Serialize(const Editor::NodeEditor& a_nodeEditor) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_nodeEditorAllocator = a_nodeEditor.GetREFNodeEditorAllocator();

	l_rootJson[k_nodeEditorAllocatorJsonKey] = l_nodeEditorAllocator.Serialize();
	l_rootJson[k_linkDataListJsonKey]        = SerializeLinkDataList          (a_nodeEditor);

	return l_rootJson;
}

void FWK::Converter::NodeEditorJsonConverter::DeserializeLinkDataList(const nlohmann::json& a_rootJson, Editor::NodeEditor& a_nodeEditor) const
{
	if (a_rootJson.is_null() ||
		!Utility::IsJsonArray(a_rootJson))
	{
		return;
	}

	for (const auto& l_json : a_rootJson)
	{
		if (l_json.is_null())
		{
			continue;
		}

		Struct::NodeEditorLinkData l_linkData = {};

		l_linkData.m_linkID      = l_json.value(k_linkIDJsonKey,      Constant::k_invalidNodeEditorID);
		l_linkData.m_outputPinID = l_json.value(k_outputPinIDJsonKey, Constant::k_invalidNodeEditorID);
		l_linkData.m_inputPinID  = l_json.value(k_inputPinIDJsonKey,  Constant::k_invalidNodeEditorID);

		a_nodeEditor.AddLink(l_linkData);
	}
}

nlohmann::json FWK::Converter::NodeEditorJsonConverter::SerializeLinkDataList(const Editor::NodeEditor& a_nodeEditor) const
{
	auto l_rootJsonArray = nlohmann::json::array();

	for (const auto& l_linkData : a_nodeEditor.GetREFLinkDataList())
	{
		nlohmann::json l_json = {};

		l_json[k_linkIDJsonKey]      = l_linkData.m_linkID;
		l_json[k_outputPinIDJsonKey] = l_linkData.m_outputPinID;
		l_json[k_inputPinIDJsonKey]  = l_linkData.m_inputPinID;

		l_rootJsonArray.emplace_back(l_json);
	}

	return l_rootJsonArray;
}