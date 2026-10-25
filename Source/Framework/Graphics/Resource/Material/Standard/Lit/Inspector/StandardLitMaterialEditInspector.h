#pragma once

namespace FWK::Graphics
{
    class StandardLitMaterial;
}

namespace FWK::Graphics
{
    // StandardLitMaterial固有の値をInspectorで編集するクラス
    // 型名、ファイルパス、保存ボタンの表示はMaterialBaseEditInspectorが行う
    class StandardLitMaterialEditInspector final
    {
    public:

         StandardLitMaterialEditInspector() = default;
        ~StandardLitMaterialEditInspector() = default;

        // 値が変わった場合はtrueを返す
        bool EditInspector(StandardLitMaterial& a_standardLitMaterial) const;

    private:

        static constexpr float k_minFactor = 0.0F;
        static constexpr float k_maxFactor = 1.0F;

        static constexpr std::string_view k_baseColorFactorLabel  = "ベースカラー";
        static constexpr std::string_view k_roughnessFactorLabel  = "ラフネス";
        static constexpr std::string_view k_metallicFactorLabel   = "メタリック";
        static constexpr std::string_view k_baseColorTextureLabel = "ベースカラーテクスチャ : %s";
        static constexpr std::string_view k_normalTextureLabel    = "法線テクスチャ : %s";
        static constexpr std::string_view k_roughnessTextureLabel = "ラフネステクスチャ : %s";
        static constexpr std::string_view k_metallicTextureLabel  = "メタリックテクスチャ : %s";
    };
}