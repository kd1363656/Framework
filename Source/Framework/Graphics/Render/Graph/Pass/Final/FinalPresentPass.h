#pragma once

namespace FWK::Graphics
{
	class FinalPresentPass final : public RenderGraphPassBase
	{
	public:

		 FinalPresentPass();
		~FinalPresentPass() override;

		void Execute(const ResourceContext& a_resourceContext, Renderer& a_renderer, RenderGraph& a_renderGraph) override;

	private:

		FWK_DEFINE_TYPE_INFO(FinalPresentPass, RenderGraphPassBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::RenderGraphPassUniqueFactory, FWK::Graphics::FinalPresentPass)