#pragma once

namespace FWK
{
	class AssetFilePathRegistry;
}

namespace FWK::Converter
{
	class AssetFilePathRegistryJsonConverter
	{
	public:

		 AssetFilePathRegistryJsonConverter() = default;
		~AssetFilePathRegistryJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, AssetFilePathRegistry& a_assetFilePathRegistry) const;

		nlohmann::json Serialize(const AssetFilePathRegistry& a_assetFilePathRegistry) const;

	private:

		void DeserializeAssetFilePathToUUIDMap(const nlohmann::json& a_rootJson, AssetFilePathRegistry& a_assetFilePathRegistry) const;
		
		nlohmann::json SerializeAssetFilePathToUUIDMap(const AssetFilePathRegistry& a_assetFilePathRegistry) const;
		
		static constexpr std::string_view k_assetFilePathToUUIDMapJsonKey = "AssetFilePathToUUIDMap";
		static constexpr std::string_view k_filePathJsonKey               = "FilePath";
		static constexpr std::string_view k_assetUUIDJsonKey              = "AssetUUID";
	};
}