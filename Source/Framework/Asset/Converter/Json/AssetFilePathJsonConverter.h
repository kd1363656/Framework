#pragma once

namespace FWK
{
    class AssetFilePath;
}

namespace FWK::Converter
{
    class AssetFilePathJsonConverter final
    {
    public:

         AssetFilePathJsonConverter() = default;
        ~AssetFilePathJsonConverter() = default;

        void Deserialize(const nlohmann::json& a_rootJson, AssetFilePath& a_assetFilePath) const;

        nlohmann::json Serialize(const AssetFilePath& a_assetFilePath) const;

    private:

        static constexpr std::string_view k_assetFilePathUUIDJsonKey = "AssetFilePathUUID";
    };
}