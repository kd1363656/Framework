#pragma once

namespace FWK::Graphics
{
	class SkeletalAnimationModelStandardLitPerObjectDrawRequest final : public SkeletalAnimationModelStandardPerObjectDrawRequestBase
	{
	public:

		 SkeletalAnimationModelStandardLitPerObjectDrawRequest()          = default;
		~SkeletalAnimationModelStandardLitPerObjectDrawRequest() override = default;

		FWK_DEFINE_TYPE_INFO(SkeletalAnimationModelStandardLitPerObjectDrawRequest, SkeletalAnimationModelStandardPerObjectDrawRequestBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DrawRequestPerObjectSharedFactory, FWK::Graphics::SkeletalAnimationModelStandardLitPerObjectDrawRequest)