#pragma once

namespace FWK
{
    class AssetFilePath
    {
    public:

         AssetFilePath() = default;
        ~AssetFilePath() = default;
    
        void Deserialize(const nlohmann::json& a_rootJson);

        nlohmann::json Serialize() const;

        void EditInspector();

        void SetAllowedFileExtension(const std::filesystem::path& a_set) { m_allowedFileExtension = a_set; }
        void SetAssetFilePath       (const std::filesystem::path& a_set) { m_assetFilePath        = a_set; }

        void SetAssetFilePathUUID(const boost::uuids::uuid& a_set) { m_assetFilePathUUID = a_set; }

        void SetIsFilePathChangedDirty(const bool a_set) { m_isFilePathChangedDirty = a_set; }

        const auto& GetREFAssetFilePath() const { return m_assetFilePath; }

        const auto& GetREFAssetFilePathUUID() const { return m_assetFilePathUUID; }

        bool GetVALIsFilePathChangedDirty() const { return m_isFilePathChangedDirty; }

    private:
    

        std::filesystem::path m_allowedFileExtension = {};
        std::filesystem::path m_assetFilePath        = {};

        Converter::AssetFilePathJsonConverter m_jsonConverter = {};

        AssetFilePathInspector m_inspector = {};

        boost::uuids::uuid m_assetFilePathUUID = {};

        bool m_isFilePathChangedDirty = false;
    };
}