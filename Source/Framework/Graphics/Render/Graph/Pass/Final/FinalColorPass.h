#pragma once

namespace FWK::Graphics
{
	class FinalColorPass final : public RenderGraphPassBase
	{
	public:

		 FinalColorPass();
		~FinalColorPass() override;

		void Execute(const ResourceContext&, Renderer& a_renderer, RenderGraph& a_renderGraph) override;

		FWK_DEFINE_TYPE_INFO(FinalColorPass, RenderGraphPassBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::RenderGraphPassUniqueFactory, FWK::Graphics::FinalColorPass)