#pragma once

namespace FWK::Graphics
{
	class SkeletalAnimationBoneMatrixBufferUploader final : public DynamicConstantBufferUploaderBase<TypeAlias::Math::Matrix>
	{
	public:

		 SkeletalAnimationBoneMatrixBufferUploader()          = default;
		~SkeletalAnimationBoneMatrixBufferUploader() override = default;

		FWK_DEFINE_TYPE_INFO(SkeletalAnimationBoneMatrixBufferUploader, DynamicBufferUploaderBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DynamicBufferSharedFactory, FWK::Graphics::SkeletalAnimationBoneMatrixBufferUploader)