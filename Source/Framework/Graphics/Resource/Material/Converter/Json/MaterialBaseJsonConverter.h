#pragma once

namespace FWK::Graphics
{
    class MaterialBase;
}

namespace FWK::Converter
{
    // MaterialBaseが持つ共通部分(Materialの型名)をjsonへ変換するクラス
    // 派生クラス固有の値は、派生クラスごとのJsonConverter(例 : StandardLitMaterialJsonConverter)が
    // このクラスの結果に書き足す
    class MaterialBaseJsonConverter final
    {
    public:

         MaterialBaseJsonConverter() = default;
        ~MaterialBaseJsonConverter() = default;

        // jsonに保存された型名がMaterial自身の型名と違う場合は、別の型の値を読まないようにfalseを返す
        bool Deserialize(const nlohmann::json& a_rootJson, const Graphics::MaterialBase& a_materialBase) const;

        nlohmann::json Serialize(const Graphics::MaterialBase& a_materialBase) const;

    private:

        static constexpr std::string_view k_materialTypeJsonKey = "MaterialType";
    };
}