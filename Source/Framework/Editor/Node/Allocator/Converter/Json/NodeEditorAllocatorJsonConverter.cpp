#include "NodeEditorAllocatorJsonConverter.h"

void FWK::Converter::NodeEditorAllocatorJsonConverter::Deserialize(const nlohmann::json& a_rootJson, NodeEditorAllocator& a_nodeEditorAllocator) const
{
	if (a_rootJson.is_null()) { return; }

	auto& l_isAllocatedList = a_nodeEditorAllocator.GetMutableREFIsAllocatedList();

	l_isAllocatedList = a_rootJson.value(k_isAllocatedListJsonKey, std::vector<bool>());
}

nlohmann::json FWK::Converter::NodeEditorAllocatorJsonConverter::Serialize(const NodeEditorAllocator& a_nodeEditorAllocator) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_isAllocatedListJsonKey] = a_nodeEditorAllocator.GetREFIsAllocatedList();
	
	return l_rootJson;
}