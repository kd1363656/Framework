#pragma once

namespace FWK::Graphics
{
	class RenderGraphBackBufferClearPass final : public RenderGraphPassBase
	{
	public:

		 RenderGraphBackBufferClearPass();
		~RenderGraphBackBufferClearPass() override;

		void Execute(const ResourceContext& a_resourceContext, Renderer& a_renderer) override;

	private:

		static constexpr TypeAlias::Math::Color k_backBufferClearColor =
		{
			1.0F,
			0.80F,
			1.0F,
			1.0F
		};

		FWK_DEFINE_TYPE_INFO(RenderGraphBackBufferClearPass, RenderGraphPassBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::RenderGraphPassUniquePTRFactory, FWK::Graphics::RenderGraphBackBufferClearPass)