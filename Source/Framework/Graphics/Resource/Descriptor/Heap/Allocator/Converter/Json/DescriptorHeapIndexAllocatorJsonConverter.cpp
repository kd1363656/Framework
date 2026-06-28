#include "DescriptorHeapIndexAllocatorJsonConverter.h"

void FWK::Converter::DescriptorHeapIndexAllocatorJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::DescriptorHeapIndexAllocator& a_descriptorHeapIndexAllocator) const
{
	if (a_rootJson.is_null()) { return; }

	const auto l_capacity = a_rootJson.value(k_capacityJsonKey, Graphics::DescriptorHeapIndexAllocator::GetVALDefaultCapacity());

	a_descriptorHeapIndexAllocator.SetCapacity(l_capacity);
}

nlohmann::json FWK::Converter::DescriptorHeapIndexAllocatorJsonConverter::Serialize(const Graphics::DescriptorHeapIndexAllocator& a_descriptorHeapIndexAllocator) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_capacityJsonKey] = a_descriptorHeapIndexAllocator.GetVALCapacity();

	return l_rootJson;
}