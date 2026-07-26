#pragma once

namespace FWK::Graphics
{
	class SkeletalAnimationModelStandardUnLitPass final : public RenderGraphPassBase
	{
	public:

		 SkeletalAnimationModelStandardUnLitPass();
		~SkeletalAnimationModelStandardUnLitPass() override;

		void Execute(const ResourceContext&, Renderer& a_renderer, RenderGraph& a_renderGraph) override;

		FWK_DEFINE_TYPE_INFO(SkeletalAnimationModelStandardUnLitPass, RenderGraphPassBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::RenderGraphPassUniqueFactory, FWK::Graphics::SkeletalAnimationModelStandardUnLitPass)