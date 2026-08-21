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

		void EditInspector();

		bool ApplyAssetFilePath(const std::filesystem::path& a_set);

		void SetAllowedFileExtension(const std::filesystem::path& a_set) { m_allowedFileExtension = a_set; }
		
		void SetIsFilePathChangedDirty(const bool a_set) { m_isFilePathChangedDirty = a_set; }

		const auto& GetREFAssetFilePath() const { return m_assetFilePath; }

		bool GetVALIsFilePathChangedDirty() const { return m_isFilePathChangedDirty; }

	private:

		AssetFilePathHelperInspector m_inspector = {};

		Converter::AssetFilePathHelperJsonConverter m_jsonConverter = {};

		std::filesystem::path m_allowedFileExtension = {};
		std::filesystem::path m_assetFilePath        = {};

		bool m_isFilePathChangedDirty = false;
	};
}