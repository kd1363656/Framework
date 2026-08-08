#pragma once

namespace FWK::Utility
{
	class AssetFilePathHelper final
	{
	public:

		 AssetFilePathHelper() = default;
		~AssetFilePathHelper() = default;

		void Deserialize(const nlohmann::json& a_rootJson);

		nlohmann::json Serialize() const;

		void SetAllowedFileExtension(const std::filesystem::path& a_set) { m_allowedFileExtension = a_set; }
		void SetAssetFilePath       (const std::filesystem::path& a_set) { m_assetFilePath        = a_set; }

		const auto& GetREFAssetFilePath() const { return m_assetFilePath; }

	private:

		Converter::AssetFilePathHelperJsonConverter m_jsonConverter = {};

		std::filesystem::path m_allowedFileExtension = {};
		std::filesystem::path m_assetFilePath        = {};
	};
}