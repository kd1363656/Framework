#pragma once

namespace FWK::Graphics
{
	class SkeletalAnimationBoneMatrixPerObjectDynamicConstantBufferUploader final : public DynamicConstantBufferUploaderBase<Struct::CBSkeletalAnimationBoneMatrix>
	{
	public:

		 SkeletalAnimationBoneMatrixPerObjectDynamicConstantBufferUploader()          = default;
		~SkeletalAnimationBoneMatrixPerObjectDynamicConstantBufferUploader() override = default;

		FWK_DEFINE_TYPE_INFO(SkeletalAnimationBoneMatrixPerObjectDynamicConstantBufferUploader, DynamicBufferUploaderBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DynamicBufferSharedFactory, FWK::Graphics::SkeletalAnimationBoneMatrixPerObjectDynamicConstantBufferUploader)