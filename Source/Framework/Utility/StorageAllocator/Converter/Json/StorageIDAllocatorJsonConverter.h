#pragma once

namespace FWK::Utility
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

		void Deserialize(const nlohmann::json& a_rootJson, Utility::StorageIDAllocator& a_storageIDAllocator) const;

		nlohmann::json Serialize(const Utility::StorageIDAllocator& a_storageIDAllocator) const;

	private:

		static constexpr std::string_view k_storageIDAllocatorCapacityJsonKey = "StorageIDAllocatorCapacity";
	};
}