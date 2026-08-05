#include "PrefabInstanceNUMAllocatorJsonConverter.h"

void FWK::Converter::PrefabInstanceNUMAllocatorJsonConverter::Deserialize(const nlohmann::json& a_rootJson, PrefabInstanceNUMAllocator& a_prefabInstanceNUMAllocator) const
{
	if (a_rootJson.is_null()) { return; }

	auto& l_isAllocatedList = a_prefabInstanceNUMAllocator.GetMutableREFIsAllocatedList();

	l_isAllocatedList = a_rootJson.value(k_isAllocatedListJsonKey, std::vector<bool>());
}

nlohmann::json FWK::Converter::PrefabInstanceNUMAllocatorJsonConverter::Serialize(const PrefabInstanceNUMAllocator& a_prefabInstanceNUMAllocator) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_isAllocatedListJsonKey] = a_prefabInstanceNUMAllocator.GetREFIsAllocatedList();

	return l_rootJson;
}