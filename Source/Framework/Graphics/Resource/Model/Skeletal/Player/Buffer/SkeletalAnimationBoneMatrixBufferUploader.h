#pragma once

namespace FWK::Graphics
{
	class SkeletalAnimationBoneMatrixBufferUploader final : public DynamicBufferUploaderBase
	{
	public:

		 SkeletalAnimationBoneMatrixBufferUploader();
		~SkeletalAnimationBoneMatrixBufferUploader() override;

		bool Create(const Device& a_device) override;

		bool Write(const std::vector<TypeAlias::Math::Matrix>& a_boneMatrixList);

		FWK_DEFINE_TYPE_INFO(SkeletalAnimationBoneMatrixBufferUploader, DynamicBufferUploaderBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DynamicBufferSharedFactory, FWK::Graphics::SkeletalAnimationBoneMatrixBufferUploader)