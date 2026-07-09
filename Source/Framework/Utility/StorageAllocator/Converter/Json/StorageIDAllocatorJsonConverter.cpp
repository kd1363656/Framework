#include "StorageIDAllocatorJsonConverter.h"

void FWK::Converter::StorageIDAllocatorJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Utility::StorageIDAllocator& a_storageIDAllocator) const
{
	if (a_rootJson.is_null()) { return; }

	const auto l_storageIDCapacity = a_rootJson.value(k_storageIDAllocatorCapacityJsonKey, a_storageIDAllocator.k_defaultCreateStorageIDCapacity);

	a_storageIDAllocator.SetStorageIDCapacity(l_storageIDCapacity);
}

nlohmann::json FWK::Converter::StorageIDAllocatorJsonConverter::Serialize(const Utility::StorageIDAllocator& a_storageIDAllocator) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_storageIDAllocatorCapacityJsonKey] = a_storageIDAllocator.GetVALStorageIDCapacity();

	return l_rootJson;
}