#pragma once

namespace FWK::Graphics
{
    class StandardUnLitMaterial;
}

namespace FWK::Converter
{
    // StandardUnLitMaterialの値をjsonへ変換するクラス
    // MaterialBaseの共通部分(型名)はMaterialBaseJsonConverterに任せ、その結果に固有の値を書き足す
    class StandardUnLitMaterialJsonConverter final
    {
    public:

         StandardUnLitMaterialJsonConverter() = default;
        ~StandardUnLitMaterialJsonConverter() = default;

        bool Deserialize(const nlohmann::json& a_rootJson, Graphics::StandardUnLitMaterial& a_standardUnLitMaterial) const;

        nlohmann::json Serialize(const Graphics::StandardUnLitMaterial& a_standardUnLitMaterial) const;

    private:

        static constexpr std::string_view k_baseColorFactorJsonKey          = "BaseColorFactor";
        static constexpr std::string_view k_baseColorTextureFilePathJsonKey = "BaseColorTextureFilePath";
    };
}