#pragma once

namespace FWK::Graphics
{
	class SkeletalAnimationBoneMatrixBufferUploader final : public DynamicBufferUploaderBase
	{
	public:

		 SkeletalAnimationBoneMatrixBufferUploader();
		~SkeletalAnimationBoneMatrixBufferUploader() override;

		SkeletalAnimationBoneMatrixBufferUploader(const SkeletalAnimationBoneMatrixBufferUploader&)  = delete;
		SkeletalAnimationBoneMatrixBufferUploader(      SkeletalAnimationBoneMatrixBufferUploader&& a_other) noexcept;

		SkeletalAnimationBoneMatrixBufferUploader& operator=(const SkeletalAnimationBoneMatrixBufferUploader&)  = delete;
		SkeletalAnimationBoneMatrixBufferUploader& operator=(      SkeletalAnimationBoneMatrixBufferUploader&&) = delete;

		bool Create(const Device& a_device) override;

		bool Write(const std::vector<TypeAlias::Math::Matrix>& a_boneMatrixList);

		FWK_DEFINE_TYPE_INFO(SkeletalAnimationBoneMatrixBufferUploader, DynamicBufferUploaderBase)
	};
}