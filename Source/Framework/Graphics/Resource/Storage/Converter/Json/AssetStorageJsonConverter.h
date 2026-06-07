#pragma once

namespace FWK::Graphics
{
	template <Concept::IsDerivedAssetRecordBaseConcept RecordType>
	class AssetStorage;
}

namespace FWK::Converter
{	
	template <Concept::IsDerivedAssetRecordBaseConcept RecordType>
	class AssetStorageJsonConverter final
	{
	public:

		 AssetStorageJsonConverter() = default;
		~AssetStorageJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Graphics::AssetStorage<RecordType>& a_assetStorage) const
		{
			if (a_rootJson.is_null())									   { return; }
			if (!a_rootJson.contains(k_storageIDAllocatorCapacityJsonKey)) { return; }
			
			auto& l_storageIDAllocator = a_assetStorage.GetMutableREFStorageIDAllocator();

			l_storageIDAllocator.Deserialize(a_rootJson[k_storageIDAllocatorCapacityJsonKey]);
		}

		nlohmann::json Serialize(const Graphics::AssetStorage<RecordType>& a_assetStorage) const
		{
			nlohmann::json l_rootJson = {};

			const auto& l_storageIDAllocator = a_assetStorage.GetREFStorageIDAllocator();

			l_rootJson[k_storageIDAllocatorCapacityJsonKey] = l_storageIDAllocator.Serialize();

			return l_rootJson;
		}

	private:

		static constexpr std::string_view k_storageIDAllocatorCapacityJsonKey = "StorageIDAllocator";
	};
}