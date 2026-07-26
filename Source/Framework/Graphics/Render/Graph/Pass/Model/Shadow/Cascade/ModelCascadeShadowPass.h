#pragma once

namespace FWK::Graphics
{
	class ModelCascadeShadowPass final : public RenderGraphPassBase
	{
	public:

		 ModelCascadeShadowPass();
		~ModelCascadeShadowPass() override;

		void Execute(const ResourceContext& a_resourceContext, Renderer& a_renderer, RenderGraph&) override;

	private:

		FWK_DEFINE_TYPE_INFO(ModelCascadeShadowPass, RenderGraphPassBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::RenderGraphPassUniqueFactory, FWK::Graphics::ModelCascadeShadowPass)