#pragma once

namespace FWK::Graphics
{
	class CascadeShadowPass final : public RenderGraphPassBase
	{
	public:

		 CascadeShadowPass();
		~CascadeShadowPass() override;

		void Execute(const ResourceContext& a_resourceContext, Renderer& a_renderer, RenderGraph&) override;

	private:

		static constexpr UINT k_firstCascadeIndex = 0U;

		FWK_DEFINE_TYPE_INFO(CascadeShadowPass, RenderGraphPassBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::RenderGraphPassUniqueFactory, FWK::Graphics::CascadeShadowPass)