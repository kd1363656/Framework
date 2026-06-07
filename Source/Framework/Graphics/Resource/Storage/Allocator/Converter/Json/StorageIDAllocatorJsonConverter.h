#pragma once

namespace FWK::Graphics
{
	class StorageIDAllocator;
}

namespace FWK::Converter
{
	class StorageIDAllocatorJsonConverter final
	{
	public:

		 StorageIDAllocatorJsonConverter() = default;
		~StorageIDAllocatorJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Graphics::StorageIDAllocator& a_storageIDAllocator) const;

		nlohmann::json Serialize(const Graphics::StorageIDAllocator& a_storageIDAllocator) const;

	private:

		static constexpr std::string_view k_storageIDAllocatorCapacityJsonKey = "StorageIDAllocatorCapacity";
	};
}