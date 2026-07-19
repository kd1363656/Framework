#pragma once

namespace FWK::Graphics
{
	class LightPassDynamicConstantBufferUploader final : public DynamicConstantBufferFixedWritePositionUploaderBase<Struct::CBLightPass>
	{
	public:

		 LightPassDynamicConstantBufferUploader()          = default;
		~LightPassDynamicConstantBufferUploader() override = default;

		FWK_DEFINE_TYPE_INFO(LightPassDynamicConstantBufferUploader, DynamicBufferUploaderBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DynamicBufferSharedFactory, FWK::Graphics::LightPassDynamicConstantBufferUploader)