#pragma once

namespace FWK::Editor
{
	class ContentBrowserAssetRegistry final
	{
	public:

		 ContentBrowserAssetRegistry() = default;
		~ContentBrowserAssetRegistry() = default;

		void Deserialize(const nlohmann::json& a_rootJson);

		nlohmann::json Serialize() const;

		bool Add(const boost::uuids::uuid& a_assetUUID, const std::filesystem::path& a_assetFilePath);

		bool Erace(const std::filesystem::path& a_assetFilePath);

		boost::uuids::uuid FindPTRAssetUUID(const std::filesystem::path& a_assetFilePath) const;

		const auto& GetREFAssetFilePathToUUIDMap() const { return m_assetFilePathToUUIDMap; };

	private:

		std::unordered_map<std::filesystem::path, boost::uuids::uuid> m_assetFilePathToUUIDMap = {};
		std::unordered_set<boost::uuids::uuid>                        m_assetUUIDSet           = {};

		Converter::ContentBrowserAssetRegistryJsonConverter m_jsonConverter = {};
	};
}