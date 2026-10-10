#pragma once

namespace FWK::Graphics
{
    class ModelRenderTableUploadPass final : public RenderGraphPassBase
    {
    public:

         ModelRenderTableUploadPass();
        ~ModelRenderTableUploadPass() override;

        void Execute(const ResourceContext&, Renderer& a_renderer, RenderGraph&) override;

        FWK_DEFINE_TYPE_INFO(ModelRenderTableUploadPass, RenderGraphPassBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::RenderGraphPassUniqueFactory, FWK::Graphics::ModelRenderTableUploadPass)