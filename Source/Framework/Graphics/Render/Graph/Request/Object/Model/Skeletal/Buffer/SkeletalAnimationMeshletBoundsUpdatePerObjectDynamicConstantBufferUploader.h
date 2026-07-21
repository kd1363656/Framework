#pragma once

namespace FWK::Graphics
{
	class SkeletalAnimationMeshletBoundsUpdatePerObjectDynamicConstantBufferUploader final : public DynamicConstantBufferAdvancingWritePositionUploaderBase<Struct::CBSkeletalAnimationMeshletBoundsUpdatePerObject>
	{
	public:

		 SkeletalAnimationMeshletBoundsUpdatePerObjectDynamicConstantBufferUploader()          = default;
		~SkeletalAnimationMeshletBoundsUpdatePerObjectDynamicConstantBufferUploader() override = default;

		FWK_DEFINE_TYPE_INFO(SkeletalAnimationMeshletBoundsUpdatePerObjectDynamicConstantBufferUploader, DynamicBufferUploaderBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DynamicBufferSharedFactory, FWK::Graphics::SkeletalAnimationMeshletBoundsUpdatePerObjectDynamicConstantBufferUploader)