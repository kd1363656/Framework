#pragma once

namespace FWK::Graphics
{
    class ModelCascadeShadowPass final : public RenderGraphPassBase
    {
    public:

         ModelCascadeShadowPass();
        ~ModelCascadeShadowPass() override;

        void Execute(const ResourceContext& a_resourceContext, Renderer& a_renderer, RenderGraph& a_renderGraph) override;

    private:

        // FrameProfilerへ記録する、描画した三角形数(全カスケードの合計)の名前
        static constexpr std::string_view k_triangleCounterName = "Triangles : Model CascadeShadow (All Cascades)";

        // カスケードの範囲の横幅がこれ以下なら、テクセル基準のLOD選択を行わない(0除算を防ぐ)
        static constexpr float k_minOrthographicWidth = 0.0F;

        static constexpr UINT64 k_initialTriangleCount = 0ULL;

        // 影のカリングでは、ライトより手前の物も影を落とすため手前の平面を使わない
        static constexpr bool k_useNearPlaneForCulling = false;

        FWK_DEFINE_TYPE_INFO(ModelCascadeShadowPass, RenderGraphPassBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::RenderGraphPassUniqueFactory, FWK::Graphics::ModelCascadeShadowPass)