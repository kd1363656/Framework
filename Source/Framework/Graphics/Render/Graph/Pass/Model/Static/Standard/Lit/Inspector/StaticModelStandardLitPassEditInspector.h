#pragma once

namespace FWK::Graphics
{
    class StaticModelStandardLitPass;
}

namespace FWK::Graphics
{
    // StaticModelStandardLitPassの設定をエディターで編集するクラス
    class StaticModelStandardLitPassEditInspector final
    {
    public:

         StaticModelStandardLitPassEditInspector() = default;
        ~StaticModelStandardLitPassEditInspector() = default;

        // 値が変わった場合はtrueを返す
        bool EditInspector(StaticModelStandardLitPass& a_staticModelStandardLitPass) const;

    private:

        static constexpr std::string_view k_occlusionCullingLabel   = "オクルージョンカリング(Hi-Z)";
        static constexpr std::string_view k_occlusionCullingTooltip = "手前の物に完全に隠れている物を描画しません。\n半透明の物の奥を描きたい場合などはオフにします。";
    };
}