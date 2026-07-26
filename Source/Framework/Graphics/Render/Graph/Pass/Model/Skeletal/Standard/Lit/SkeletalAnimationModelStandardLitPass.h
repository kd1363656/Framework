#pragma once

namespace FWK::Graphics
{
	class SkeletalAnimationModelStandardLitPass final : public RenderGraphPassBase
	{
	public:

		 SkeletalAnimationModelStandardLitPass();
		~SkeletalAnimationModelStandardLitPass() override;

		void Execute(const ResourceContext&, Renderer& a_renderer, RenderGraph& a_renderGraph) override;

		FWK_DEFINE_TYPE_INFO(SkeletalAnimationModelStandardLitPass, RenderGraphPassBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::RenderGraphPassUniqueFactory, FWK::Graphics::SkeletalAnimationModelStandardLitPass)