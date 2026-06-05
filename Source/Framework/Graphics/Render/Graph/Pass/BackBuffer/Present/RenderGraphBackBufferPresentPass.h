#pragma once

namespace FWK::Graphics
{
	class RenderGraphBackBufferPresentPass final : public RenderGraphPassBase
	{
	public:

		 RenderGraphBackBufferPresentPass();
		~RenderGraphBackBufferPresentPass() override;

		void Execute(const ResourceContext& a_resourceContext, Renderer& a_renderer) override;

		FWK_DEFINE_TYPE_INFO(RenderGraphBackBufferPresentPass, RenderGraphPassBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::RenderGraphPassUniquePTRFactory, FWK::Graphics::RenderGraphBackBufferPresentPass)