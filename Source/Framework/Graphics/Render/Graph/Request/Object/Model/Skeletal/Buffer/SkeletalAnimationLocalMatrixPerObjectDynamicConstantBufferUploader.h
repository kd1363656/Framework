#pragma once

namespace FWK::Graphics
{
	class SkeletalAnimationLocalMatrixPerObjectDynamicConstantBufferUploader final : public DynamicConstantBufferUploaderBase<Struct::CBSkeletalAnimationLocalMatrix>
	{
	public:

		 SkeletalAnimationLocalMatrixPerObjectDynamicConstantBufferUploader()          = default;
		~SkeletalAnimationLocalMatrixPerObjectDynamicConstantBufferUploader() override = default;

		FWK_DEFINE_TYPE_INFO(SkeletalAnimationLocalMatrixPerObjectDynamicConstantBufferUploader, DynamicBufferUploaderBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DynamicBufferSharedFactory, FWK::Graphics::SkeletalAnimationLocalMatrixPerObjectDynamicConstantBufferUploader)