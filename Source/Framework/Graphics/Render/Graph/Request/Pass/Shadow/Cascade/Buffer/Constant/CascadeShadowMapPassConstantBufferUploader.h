#pragma once

namespace FWK::Graphics
{
	class CascadeShadowMapPassDynamicConstantBufferUploader final : public DynamicConstantBufferFixedWritePositionUploaderBase<Struct::CBCascadeShadowMapPass>
	{
	public:

		 CascadeShadowMapPassDynamicConstantBufferUploader()          = default;
		~CascadeShadowMapPassDynamicConstantBufferUploader() override = default;

		FWK_DEFINE_TYPE_INFO(CascadeShadowMapPassDynamicConstantBufferUploader, DynamicBufferUploaderBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DynamicBufferSharedFactory, FWK::Graphics::CascadeShadowMapPassDynamicConstantBufferUploader)