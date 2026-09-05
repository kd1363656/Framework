
#pragma once

namespace FWK
{
	class AssetFilePathRegistry final
	{
	public:

		 AssetFilePathRegistry() = default;
		~AssetFilePathRegistry() = default;

		void INIT();

		void Deserialize(const nlohmann::json& a_rootJson);

		nlohmann::json Serialize() const;

		bool Add(const std::filesystem::path& a_assetFilePath, const boost::uuids::uuid& a_assetUUID, Enum::AssetFilePathRegistryType a_assetFilePathRegisterType);

		bool ReplaceFilePath(const std::filesystem::path& a_oldAssetFilePath, const std::filesystem::path& a_newAssetFilePath);

		bool Erase(const std::filesystem::path& a_assetFilePath);

		const boost::uuids::uuid*        FindPTRAssetUUID        (const std::filesystem::path& a_assetFilePath) const;
		const Struct::AssetFilePathData* FindPTRAssetFilePathData(const boost::uuids::uuid&    a_uuid)          const;

		bool ContainsAssetUUID(const boost::uuids::uuid& a_assetUUID) const;

		const auto& GetREFAssetFilePathToUUIDMap() const { return m_assetFilePathToUUIDMap; };

	private:

		std::unordered_map<std::filesystem::path, boost::uuids::uuid>        m_assetFilePathToUUIDMap  = {};
		std::unordered_map<boost::uuids::uuid,    Struct::AssetFilePathData> m_uuidToAssetFilePathData = {};

		Converter::AssetFilePathRegistryJsonConverter m_jsonConverter = {};
	};
}