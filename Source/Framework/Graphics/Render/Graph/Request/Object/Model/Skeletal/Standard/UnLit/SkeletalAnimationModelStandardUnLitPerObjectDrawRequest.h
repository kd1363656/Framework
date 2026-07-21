#pragma once

namespace FWK::Graphics
{
	class SkeletalAnimationModelStandardUnLitPerObjectDrawRequest final : public SkeletalAnimationModelStandardPerObjectDrawRequestBase
	{
	public:

		 SkeletalAnimationModelStandardUnLitPerObjectDrawRequest()          = default;
		~SkeletalAnimationModelStandardUnLitPerObjectDrawRequest() override = default;

		FWK_DEFINE_TYPE_INFO(SkeletalAnimationModelStandardUnLitPerObjectDrawRequest, SkeletalAnimationModelStandardPerObjectDrawRequestBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DrawRequestPerObjectSharedFactory, FWK::Graphics::SkeletalAnimationModelStandardUnLitPerObjectDrawRequest)