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

        static constexpr float k_minOrthographicWidth = 0.0F;

        // 影のカリングでは、ライトより手前の物も影を落とすため手前の平面を使わない
        static constexpr bool k_useNearPlaneForCulling = false;

        FWK_DEFINE_TYPE_INFO(ModelCascadeShadowPass, RenderGraphPassBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::RenderGraphPassUniqueFactory, FWK::Graphics::ModelCascadeShadowPass)