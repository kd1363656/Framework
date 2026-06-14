#pragma once

namespace FWK::Graphics
{
	class FinalPresentPass final : public RenderGraphPassBase
	{
	public:

		 FinalPresentPass()			 = default;
		~FinalPresentPass() override = default;

		void Execute(const ResourceContext& a_resourceContext, const Renderer& a_renderer) override;

	private:

		FWK_DEFINE_TYPE_INFO(FinalPresentPass, RenderGraphPassBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::RenderGraphPassUniqueFactory, FWK::Graphics::FinalPresentPass)