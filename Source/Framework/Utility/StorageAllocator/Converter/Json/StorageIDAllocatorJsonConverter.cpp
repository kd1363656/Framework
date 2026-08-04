#include "StorageIDAllocatorJsonConverter.h"

void FWK::Converter::StorageIDAllocatorJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Utility::StorageIDAllocator& a_storageIDAllocator) const
{
	if (a_rootJson.is_null()) { return; }

	const auto l_storageIDCapacity = a_rootJson.value(k_capacityJsonKey, Utility::StorageIDAllocator::k_defaultCreateStorageIDCapacity);
	const auto l_nextIDAllocator   = a_rootJson.value(k_nextIDJsonKey,   Utility::StorageIDAllocator::k_initialNextStorageID);

	a_storageIDAllocator.SetStorageIDCapacity(l_storageIDCapacity);
	a_storageIDAllocator.SetNextStorageID    (l_nextIDAllocator);
}

nlohmann::json FWK::Converter::StorageIDAllocatorJsonConverter::Serialize(const Utility::StorageIDAllocator& a_storageIDAllocator) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_capacityJsonKey] = a_storageIDAllocator.GetVALStorageIDCapacity();
	l_rootJson[k_nextIDJsonKey]   = a_storageIDAllocator.GetVALNextStorageID    ();

	return l_rootJson;
}