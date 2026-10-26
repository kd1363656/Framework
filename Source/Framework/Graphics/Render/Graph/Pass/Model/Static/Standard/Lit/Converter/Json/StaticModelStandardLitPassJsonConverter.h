#pragma once

namespace FWK::Graphics
{
    class StaticModelStandardLitPass;
}

namespace FWK::Converter
{
    // StaticModelStandardLitPassの設定をjsonへ変換するクラス
    // (GraphicsCONFIG.jsonのRenderGraphPassListの中の、このPassの要素に書き込む)
    class StaticModelStandardLitPassJsonConverter final
    {
    public:

         StaticModelStandardLitPassJsonConverter() = default;
        ~StaticModelStandardLitPassJsonConverter() = default;

        void Deserialize(const nlohmann::json& a_rootJson, Graphics::StaticModelStandardLitPass& a_staticModelStandardLitPass) const;

        nlohmann::json Serialize(const Graphics::StaticModelStandardLitPass& a_staticModelStandardLitPass) const;

    private:

        static constexpr std::string_view k_isOcclusionCullingEnabledJsonKey = "IsOcclusionCullingEnabled";
    };
}