#pragma once

namespace FWK::Graphics
{
    class SpriteScreenPass final : public RenderGraphPassBase
    {
    public:

         SpriteScreenPass();
        ~SpriteScreenPass() override;

        void Execute(const ResourceContext&, Renderer& a_renderer, RenderGraph& a_renderGraph) override;

    private:

        FWK_DEFINE_TYPE_INFO(SpriteScreenPass, RenderGraphPassBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::RenderGraphPassUniqueFactory, FWK::Graphics::SpriteScreenPass)