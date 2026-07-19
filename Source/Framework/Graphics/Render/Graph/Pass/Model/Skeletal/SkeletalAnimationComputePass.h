#pragma once

namespace FWK::Graphics
{
	class SkeletalAnimationComputePass final : public RenderGraphPassBase
	{
	public:

		 SkeletalAnimationComputePass();
		~SkeletalAnimationComputePass() override;

		void Execute(Renderer& a_renderer, RenderGraph& a_renderGraph) override;

	private:

		static constexpr UINT64 k_boneMatrixBufferCopyDestinationOffset = 0ULL;
		static constexpr UINT64 k_boneMatrixBufferCopySourceOffset      = 0ULL;

		FWK_DEFINE_TYPE_INFO(SkeletalAnimationComputePass, RenderGraphPassBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::RenderGraphPassUniqueFactory, FWK::Graphics::SkeletalAnimationComputePass)