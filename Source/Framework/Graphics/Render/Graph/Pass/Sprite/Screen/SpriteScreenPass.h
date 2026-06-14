#pragma once

namespace FWK::Graphics
{
	class ScreenSpritePass final : public RenderGraphPassBase
	{
	public:

		 ScreenSpritePass();
		~ScreenSpritePass() override;

		void Execute(const ResourceContext& a_resourceContext, Renderer& a_renderer, RenderGraph& a_renderGraph) override;

	private:

		FWK_DEFINE_TYPE_INFO(ScreenSpritePass, RenderGraphPassBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::RenderGraphPassUniqueFactory, FWK::Graphics::ScreenSpritePass)