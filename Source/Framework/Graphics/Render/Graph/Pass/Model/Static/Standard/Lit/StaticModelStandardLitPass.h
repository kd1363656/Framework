#pragma once

namespace FWK::Graphics
{
	class StaticModelStandardLitPass final : public RenderGraphPassBase
	{
	public:

		 StaticModelStandardLitPass();
		~StaticModelStandardLitPass() override;

		void Execute(Renderer& a_renderer, RenderGraph& a_renderGraph) override;

		FWK_DEFINE_TYPE_INFO(StaticModelStandardLitPass, RenderGraphPassBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::RenderGraphPassUniqueFactory, FWK::Graphics::StaticModelStandardLitPass)