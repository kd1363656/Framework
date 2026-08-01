#pragma once

namespace FWK::Utility
{
	class AssetFilePath final
	{
	public:

		 AssetFilePath() = default;
		~AssetFilePath() = default;

		void Deserialize(const nlohmann::json& a_rootJson);

		nlohmann::json Serialize() const;

		void SetAssetFilePath(const std::filesystem::path& a_set) { m_assetFilePath = a_set; }

		const auto& GetREFAssetFilePath() const { return m_assetFilePath; }

	private:

		Converter::AssetFilePathJsonConverter m_jsonConverter = {};

		std::filesystem::path m_assetFilePath = {};
	};
}