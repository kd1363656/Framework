#pragma once

namespace FWK::Graphics
{
	template <D3D12_DESCRIPTOR_HEAP_TYPE HeapType>
	class DescriptorPool;
}

namespace FWK::Converter
{
	template <D3D12_DESCRIPTOR_HEAP_TYPE HeapType>
	class DescriptorPoolJsonConverter final
	{
	public:

		 DescriptorPoolJsonConverter() = default;
		~DescriptorPoolJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, FWK::Graphics::DescriptorPool<HeapType>& a_descriptorPool) const
		{
			if (a_rootJson.is_null()) { return; }

			auto& l_descriptorHeapIndexAllocator = a_descriptorPool.GetMutableREFDescriptorIndexAllocator();
			
			const auto& l_descriptorHeapIndexAllocatorJson = a_rootJson.value(k_descriptorHeapIndexAllocatorJsonKey, nlohmann::json{});

			if (l_descriptorHeapIndexAllocatorJson.is_null()) { return; }
			
			l_descriptorHeapIndexAllocator.Deserialize(l_descriptorHeapIndexAllocatorJson);
		}

		nlohmann::json Serialize(const FWK::Graphics::DescriptorPool<HeapType>& a_descriptorPool) const
		{
			nlohmann::json l_rootJson = {};

			const auto& l_descriptorHeapIndexAllocator = a_descriptorPool.GetREFDescriptorIndexAllocator();
			
			l_rootJson[k_descriptorHeapIndexAllocatorJsonKey] = l_descriptorHeapIndexAllocator.Serialize();

			return l_rootJson;
		}

	private:

		static constexpr std::string_view k_descriptorHeapIndexAllocatorJsonKey = "DescriptorHeapIndexAllocator";
	};
}