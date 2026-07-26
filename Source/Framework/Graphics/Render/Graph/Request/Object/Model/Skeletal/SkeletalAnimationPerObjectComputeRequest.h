#pragma once

namespace FWK::Graphics
{
	class SkeletalAnimationPerObjectComputeRequest final : public ComputeRequestPerObjectBase
	{
	public:

		 SkeletalAnimationPerObjectComputeRequest()          = default;
		~SkeletalAnimationPerObjectComputeRequest() override = default;

		void BeginFrame() override;

		void AddComputeRequest(const std::shared_ptr<SkeletalAnimationPlayer>& a_skeletalAnimationPlayer);

		const auto& GetREFSkeletalAnimationPlayerSmartPointerVectorArray() const { return m_skeletalAnimationPlayerSmartPointerVectorArray; }

	private:

		Utility::SmartPointerVectorArray<std::weak_ptr<SkeletalAnimationPlayer>> m_skeletalAnimationPlayerSmartPointerVectorArray = {};

		FWK_DEFINE_TYPE_INFO(SkeletalAnimationPerObjectComputeRequest, ComputeRequestPerObjectBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::ComputeRequestPerObjectSharedFactory, FWK::Graphics::SkeletalAnimationPerObjectComputeRequest)