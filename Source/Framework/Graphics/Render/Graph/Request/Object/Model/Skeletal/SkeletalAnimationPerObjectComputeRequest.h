#pragma once

namespace FWK::Graphics
{
	class SkeletalAnimationPerObjectComputeRequest final : public ComputeRequestPerObjectBase
	{
	public:

		 SkeletalAnimationPerObjectComputeRequest()          = default;
		~SkeletalAnimationPerObjectComputeRequest() override = default;

		void BeginFrame() override;

		void DispatchLocalMatrix(const Renderer& a_renderer, const RootSignature& a_rootSignature, const FrameResource& a_frameResource);

		void AddComputeRequest(const std::shared_ptr<SkeletalAnimationPlayer>& a_skeletalAnimationPlayer);

	private:

		static constexpr UINT k_singleDispatchThreadGroupCount = 1U;

		Utility::VectorArray<std::weak_ptr<SkeletalAnimationPlayer>> m_skeletalAnimationPlayerList = {};

		FWK_DEFINE_TYPE_INFO(SkeletalAnimationPerObjectComputeRequest, ComputeRequestPerObjectBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::ComputeRequestPerObjectSharedFactory, FWK::Graphics::SkeletalAnimationPerObjectComputeRequest)