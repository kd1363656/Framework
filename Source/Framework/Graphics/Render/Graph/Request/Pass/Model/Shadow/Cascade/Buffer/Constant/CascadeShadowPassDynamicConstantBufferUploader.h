#pragma once

namespace FWK::Graphics
{
	class CascadeShadowPassDynamicConstantBufferUploader final : public DynamicConstantBufferAdvancingWritePositionUploaderBase<Struct::CBCascadeShadowPass>
	{
	public:

		 CascadeShadowPassDynamicConstantBufferUploader()          = default;
		~CascadeShadowPassDynamicConstantBufferUploader() override = default;

		FWK_DEFINE_TYPE_INFO(CascadeShadowPassDynamicConstantBufferUploader, DynamicBufferUploaderBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DynamicBufferSharedFactory, FWK::Graphics::CascadeShadowPassDynamicConstantBufferUploader)