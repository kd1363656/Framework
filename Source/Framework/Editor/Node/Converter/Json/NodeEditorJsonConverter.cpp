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
}

nlohmann::json FWK::Converter::NodeEditorJsonConverter::Serialize(const Editor::NodeEditor& a_nodeEditor) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_nodeEditorAllocator = a_nodeEditor.GetREFNodeEditorAllocator();

	l_rootJson[k_nodeEditorAllocatorJsonKey] = l_nodeEditorAllocator.Serialize();

	return l_rootJson;
}