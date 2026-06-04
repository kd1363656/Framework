#pragma once

namespace FWK::Graphics
{
	class DescriptorHeapIndexAllocator;
}

namespace FWK::Converter
{
	class DescriptorHeapIndexAllocatorJsonConverter final
	{
	public:
		
		 DescriptorHeapIndexAllocatorJsonConverter() = default;
		~DescriptorHeapIndexAllocatorJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Graphics::DescriptorHeapIndexAllocator& a_descriptorHeapIndexAllocator) const;

		nlohmann::json Serialize(const Graphics::DescriptorHeapIndexAllocator& a_descriptorHeapIndexAllocator) const;

	private:

		static constexpr std::string_view k_descriptorHeapIndexAllocatorCapacityJsonKey = "DescriptorHeapIndexAllocatorCapacity";
	};
}