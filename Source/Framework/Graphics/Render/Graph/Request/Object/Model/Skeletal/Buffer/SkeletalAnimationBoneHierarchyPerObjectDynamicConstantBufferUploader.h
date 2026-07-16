#pragma once

namespace FWK::Graphics
{
	class SkeletalAnimationBoneHierarchyPerObjectDynamicConstantBufferUploader final : public DynamicConstantBufferUploaderBase<Struct::CBSkeletalAnimationBoneHierarchy>
	{
	public:

		 SkeletalAnimationBoneHierarchyPerObjectDynamicConstantBufferUploader()          = default;
		~SkeletalAnimationBoneHierarchyPerObjectDynamicConstantBufferUploader() override = default;

		FWK_DEFINE_TYPE_INFO(SkeletalAnimationBoneHierarchyPerObjectDynamicConstantBufferUploader, DynamicBufferUploaderBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DynamicBufferSharedFactory, FWK::Graphics::SkeletalAnimationBoneHierarchyPerObjectDynamicConstantBufferUploader)