#pragma once

namespace FWK::Graphics
{
	class StaticModelStandardUnLitPass final : public RenderGraphPassBase
	{
	public:

		 StaticModelStandardUnLitPass();
		~StaticModelStandardUnLitPass() override;

		void Execute(Renderer& a_renderer, RenderGraph& a_renderGraph) override;

		FWK_DEFINE_TYPE_INFO(StaticModelStandardUnLitPass, RenderGraphPassBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::RenderGraphPassUniqueFactory, FWK::Graphics::StaticModelStandardUnLitPass)