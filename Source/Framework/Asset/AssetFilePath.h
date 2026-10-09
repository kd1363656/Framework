#pragma once

namespace FWK
{
    class AssetFilePath final
    {
    public:

         AssetFilePath() = default;
        ~AssetFilePath() = default;

        void Deserialize(const nlohmann::json& a_rootJson);

        bool EditInspector();

        nlohmann::json Serialize() const;

        void SetAllowedType      (const Enum::AssetFilePathType a_set) { m_allowedType       = a_set; }
        void SetAssetFilePathUUID(const boost::uuids::uuid&     a_set) { m_assetFilePathUUID = a_set; }

        std::filesystem::path FetchVALFilePath() const;

        const auto& GetREFAssetFilePathUUID() const { return m_assetFilePathUUID; }

        auto GetVALAllowedType() const { return m_allowedType; }

    private:

        Converter::AssetFilePathJsonConverter m_jsonConverter = {};

        AssetFilePathInspector m_inspector = {};

        boost::uuids::uuid m_assetFilePathUUID = {};

        Enum::AssetFilePathType m_allowedType = Enum::AssetFilePathType::Invalid;
    };
}