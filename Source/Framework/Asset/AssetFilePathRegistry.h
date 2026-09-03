
#pragma once

namespace FWK
{
	class AssetFilePathRegistry final
	{
	public:

		 AssetFilePathRegistry() = default;
		~AssetFilePathRegistry() = default;

		void Deserialize(const nlohmann::json& a_rootJson);

		nlohmann::json Serialize() const;

		bool Add(const boost::uuids::uuid& a_assetUUID, const std::filesystem::path& a_assetFilePath);

		bool Erase(const std::filesystem::path& a_assetFilePath);

		boost::uuids::uuid FindVALAssetUUID(const std::filesystem::path& a_assetFilePath) const;

		bool ContainsAssetUUID(const boost::uuids::uuid& a_assetUUID) const;

		const auto& GetREFAssetFilePathToUUIDMap() const { return m_assetFilePathToUUIDMap; };

	private:

		std::unordered_map<std::filesystem::path, boost::uuids::uuid> m_assetFilePathToUUIDMap = {};
		std::unordered_set<boost::uuids::uuid>                        m_assetUUIDSet           = {};

		Converter::AssetFilePathRegistryJsonConverter m_jsonConverter = {};
	};
}