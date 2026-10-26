#pragma once

namespace FWK::Graphics
{
    class RitchToonMaterial;
}

namespace FWK::Converter
{
    // RitchToonMaterialの値をjsonへ変換するクラス
    // MaterialBaseの共通部分(型名)はMaterialBaseJsonConverterに任せ、その結果に固有の値を書き足す
    class RitchToonMaterialJsonConverter final
    {
    public:

         RitchToonMaterialJsonConverter() = default;
        ~RitchToonMaterialJsonConverter() = default;

        bool Deserialize(const nlohmann::json& a_rootJson, Graphics::RitchToonMaterial& a_ritchToonMaterial) const;

        nlohmann::json Serialize(const Graphics::RitchToonMaterial& a_ritchToonMaterial) const;

    private:

        static constexpr std::string_view k_baseColorFactorJsonKey             = "BaseColorFactor";
        static constexpr std::string_view k_shadowColorJsonKey                 = "ShadowColor";
        static constexpr std::string_view k_matcapColorJsonKey                 = "MatcapColor";
        static constexpr std::string_view k_specularColorJsonKey               = "SpecularColor";
        static constexpr std::string_view k_emissionColorJsonKey               = "EmissionColor";
        static constexpr std::string_view k_outlineColorJsonKey                = "OutlineColor";
        static constexpr std::string_view k_shadowBorderJsonKey                = "ShadowBorder";
        static constexpr std::string_view k_shadowBlurJsonKey                  = "ShadowBlur";
        static constexpr std::string_view k_normalStrengthJsonKey              = "NormalStrength";
        static constexpr std::string_view k_smoothnessJsonKey                  = "Smoothness";
        static constexpr std::string_view k_outlineWidthJsonKey                = "OutlineWidth";
        static constexpr std::string_view k_emissionBaseColorBlendJsonKey      = "EmissionBaseColorBlend";
        static constexpr std::string_view k_emissionReplaceRateJsonKey         = "EmissionReplaceRate";
        static constexpr std::string_view k_baseColorTextureFilePathJsonKey    = "BaseColorTextureFilePath";
        static constexpr std::string_view k_normalTextureFilePathJsonKey       = "NormalTextureFilePath";
        static constexpr std::string_view k_shadowMaskTextureFilePathJsonKey   = "ShadowMaskTextureFilePath";
        static constexpr std::string_view k_matcapTextureFilePathJsonKey       = "MatcapTextureFilePath";
        static constexpr std::string_view k_matcapMaskTextureFilePathJsonKey   = "MatcapMaskTextureFilePath";
        static constexpr std::string_view k_metallicMaskTextureFilePathJsonKey = "MetallicMaskTextureFilePath";
        static constexpr std::string_view k_emissionMaskTextureFilePathJsonKey = "EmissionMaskTextureFilePath";
        static constexpr std::string_view k_outlineMaskTextureFilePathJsonKey  = "OutlineMaskTextureFilePath";
    };
}