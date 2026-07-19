#pragma once

namespace FWK::Graphics
{
	class SkeletalAnimationVertexSkinningPerObjectDynamicConstantBufferUploader final : public DynamicConstantBufferAdvancingWritePositionUploaderBase<Struct::CBSkeletalAnimationVertexSkinning>
	{
	public:

		 SkeletalAnimationVertexSkinningPerObjectDynamicConstantBufferUploader()          = default;
		~SkeletalAnimationVertexSkinningPerObjectDynamicConstantBufferUploader() override = default;

		FWK_DEFINE_TYPE_INFO(SkeletalAnimationVertexSkinningPerObjectDynamicConstantBufferUploader, DynamicBufferUploaderBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DynamicBufferSharedFactory, FWK::Graphics::SkeletalAnimationVertexSkinningPerObjectDynamicConstantBufferUploader)