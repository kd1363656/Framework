#pragma once

namespace FWK::Graphics
{
    class RitchToonMaterial;
}

namespace FWK::Graphics
{
    // RitchToonMaterial固有の値をInspectorで編集するクラス
    // 型名、ファイルパス、保存ボタンの表示はMaterialBaseEditInspectorが行う
    class RitchToonMaterialEditInspector final
    {
    public:

         RitchToonMaterialEditInspector() = default;
        ~RitchToonMaterialEditInspector() = default;

        // 値が変わった場合はtrueを返す
        bool EditInspector(RitchToonMaterial& a_ritchToonMaterial) const;

    private:

        // 色、値、テクスチャを、それぞれまとめて編集(表示)する
        bool EditColor  (      RitchToonMaterial& a_ritchToonMaterial) const;
        bool EditFactor (      RitchToonMaterial& a_ritchToonMaterial) const;
        void ShowTexture(const RitchToonMaterial& a_ritchToonMaterial) const;

        // マットキャップ、光沢、発光の色は1.0Fより明るくできる(強さも色で決めるため)
        static constexpr ImGuiColorEditFlags k_hdrColorEditFlags = ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float;

        static constexpr float k_minFactor            = 0.0F;
        static constexpr float k_maxFactor            = 1.0F;
        static constexpr float k_maxOutlineWidthMeter = 0.02F;

        static constexpr std::string_view k_outlineWidthFormat = "%.4f m";

        static constexpr std::string_view k_baseColorFactorLabel     = "ベースカラー";
        static constexpr std::string_view k_shadowColorLabel         = "影色";
        static constexpr std::string_view k_matcapColorLabel         = "マットキャップの色";
        static constexpr std::string_view k_specularColorLabel       = "光沢の色";
        static constexpr std::string_view k_emissionColorLabel       = "発光色";
        static constexpr std::string_view k_outlineColorLabel        = "輪郭線の色";
        static constexpr std::string_view k_shadowBorderLabel        = "影の境界";
        static constexpr std::string_view k_shadowBlurLabel          = "影のぼかし";
        static constexpr std::string_view k_normalStrengthLabel      = "法線の強さ";
        static constexpr std::string_view k_smoothnessLabel          = "滑らかさ";
        static constexpr std::string_view k_outlineWidthLabel        = "輪郭線の太さ(1m先から見た時)";
        static constexpr std::string_view k_emissionBaseColorLabel   = "発光にメインテクスチャの色を混ぜる";
        static constexpr std::string_view k_emissionReplaceLabel     = "発光で元の色を置き換える";
        static constexpr std::string_view k_baseColorTextureLabel    = "メインテクスチャ : %s";
        static constexpr std::string_view k_normalTextureLabel       = "法線テクスチャ : %s";
        static constexpr std::string_view k_shadowMaskTextureLabel   = "影マスク : %s";
        static constexpr std::string_view k_matcapTextureLabel       = "マットキャップ : %s";
        static constexpr std::string_view k_matcapMaskTextureLabel   = "マットキャップマスク : %s";
        static constexpr std::string_view k_metallicMaskTextureLabel = "メタリックマスク : %s";
        static constexpr std::string_view k_emissionMaskTextureLabel = "発光マスク : %s";
        static constexpr std::string_view k_outlineMaskTextureLabel  = "輪郭線マスク : %s";
    };
}