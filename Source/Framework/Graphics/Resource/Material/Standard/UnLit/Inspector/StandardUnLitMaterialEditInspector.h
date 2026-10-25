#pragma once

namespace FWK::Graphics
{
    class StandardUnLitMaterial;
}

namespace FWK::Graphics
{
    class StandardUnLitMaterialEditInspector final
    {
    public:

         StandardUnLitMaterialEditInspector() = default;
        ~StandardUnLitMaterialEditInspector() = default;

        // 値が変わった場合はtrueを返す
        bool EditInspector(StandardUnLitMaterial& a_standardUnLitMaterial) const;

    private:

        static constexpr std::string_view k_baseColorFactorLabel  = "ベースカラー";
        static constexpr std::string_view k_baseColorTextureLabel = "ベースカラーテクスチャ : %s";
    };
}