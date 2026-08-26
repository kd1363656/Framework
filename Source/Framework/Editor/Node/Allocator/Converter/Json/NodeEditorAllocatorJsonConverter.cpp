#include "NodeEditorAllocatorJsonConverter.h"

void FWK::Converter::NodeEditorAllocatorJsonConverter::Deserialize(const nlohmann::json& a_rootJson, NodeEditorAllocator& a_nodeEditorAllocator) const
{
	if (a_rootJson.is_null()) { return; }

	const auto l_nodeEditorIDCapacity = a_rootJson.value(k_capacityJsonKey, k_nodeEditorIDCapacity);

	a_nodeEditorAllocator.SetNodeEditorIDCapacity(l_nodeEditorIDCapacity);
}

nlohmann::json FWK::Converter::NodeEditorAllocatorJsonConverter::Serialize(const NodeEditorAllocator& a_nodeEditorAllocator) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_capacityJsonKey] = a_nodeEditorAllocator.GetVALNodeEditorIDCapacity();

	return l_rootJson;
}