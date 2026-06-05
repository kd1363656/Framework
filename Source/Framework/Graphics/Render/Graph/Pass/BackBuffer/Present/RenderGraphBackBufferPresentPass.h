#pragma once

namespace FWK::Graphics
{
	class RenderGraphBackBufferPresentPass final : public RenderGraphPassBase
	{
	public:

		 RenderGraphBackBufferPresentPass();
		~RenderGraphBackBufferPresentPass() override;

		void Execute(const ResourceContext& a_resourceContext, Renderer& a_renderer) override;
	};
}