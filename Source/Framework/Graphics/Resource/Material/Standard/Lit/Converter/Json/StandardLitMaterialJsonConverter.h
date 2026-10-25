#pragma once

namespace FWK::Graphics
{
    class StandardLitMaterial;
}

namespace FWK::Converter
{
    // StandardLitMaterialの値をjsonへ変換するクラス
    // MaterialBaseの共通部分(型名)はMaterialBaseJsonConverterに任せ、その結果に固有の値を書き足す
    class StandardLitMaterialJsonConverter final
    {
    public:

         StandardLitMaterialJsonConverter() = default;
        ~StandardLitMaterialJsonConverter() = default;

        bool Deserialize(const nlohmann::json& a_rootJson, Graphics::StandardLitMaterial& a_standardLitMaterial) const;

        nlohmann::json Serialize(const Graphics::StandardLitMaterial& a_standardLitMaterial) const;

    private:

        static constexpr std::string_view k_baseColorFactorJsonKey          = "BaseColorFactor";
        static constexpr std::string_view k_roughnessFactorJsonKey          = "RoughnessFactor";
        static constexpr std::string_view k_metallicFactorJsonKey           = "MetallicFactor";
        static constexpr std::string_view k_baseColorTextureFilePathJsonKey = "BaseColorTextureFilePath";
        static constexpr std::string_view k_normalTextureFilePathJsonKey    = "NormalTextureFilePath";
        static constexpr std::string_view k_roughnessTextureFilePathJsonKey = "RoughnessTextureFilePath";
        static constexpr std::string_view k_metallicTextureFilePathJsonKey  = "MetallicTextureFilePath";
    };
}