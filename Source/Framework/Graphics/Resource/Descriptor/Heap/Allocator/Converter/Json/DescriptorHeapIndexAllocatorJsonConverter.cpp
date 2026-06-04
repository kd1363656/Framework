#include "DescriptorHeapIndexAllocatorJsonConverter.h"

void FWK::Converter::DescriptorHeapIndexAllocatorJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::DescriptorHeapIndexAllocator& a_descriptorHeapIndexAllocator) const
{
	if (a_rootJson.is_null()) { return; }

	const auto l_descriptorHeapIndexAllocatorCapacity = a_rootJson.value(k_descriptorHeapIndexAllocatorCapacityJsonKey, Graphics::DescriptorHeapIndexAllocator::GetVALDefaultIndexCapacity());

	a_descriptorHeapIndexAllocator.SetIndexCapacity(l_descriptorHeapIndexAllocatorCapacity);
}

nlohmann::json FWK::Converter::DescriptorHeapIndexAllocatorJsonConverter::Serialize(const Graphics::DescriptorHeapIndexAllocator& a_descriptorHeapIndexAllocator) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_descriptorHeapIndexAllocatorCapacityJsonKey] = a_descriptorHeapIndexAllocator.GetVALIndexCapacity();

	return l_rootJson;
}