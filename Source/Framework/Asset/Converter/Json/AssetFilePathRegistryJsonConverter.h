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

		void DeserializeFilePathRegistryMap(const nlohmann::json& a_rootJson, AssetFilePathRegistry& a_assetFilePathRegistry) const;
		
		nlohmann::json SerializeFilePathRegistryMap(const AssetFilePathRegistry& a_assetFilePathRegistry) const;
		
		static constexpr std::string_view k_filePathRegistryMapJsonKey  = "FilePathRegistryMap";
		static constexpr std::string_view k_filePathJsonKey             = "FilePath";
		static constexpr std::string_view k_uuidJsonKey                 = "UUID";
		static constexpr std::string_view k_filePathRegistryTypeJsonKey = "FilePathRegistryType";
	};
}