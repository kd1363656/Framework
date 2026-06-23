#pragma once

namespace FWK::Graphics
{
	class LightPassConstantBufferUploader final : public ConstantBufferUploader<Struct::CBLightPass>
	{
	public:

		 LightPassConstantBufferUploader()          = default;
		~LightPassConstantBufferUploader() override = default;

		FWK_DEFINE_TYPE_INFO(LightPassConstantBufferUploader, ConstantBufferUploaderBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::ConstantBufferSharedFactory, FWK::Graphics::LightPassConstantBufferUploader)