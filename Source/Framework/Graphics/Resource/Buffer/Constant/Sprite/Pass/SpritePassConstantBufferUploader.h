#pragma once

namespace FWK::Graphics
{
	class SpritePassConstantBufferUploader final : public ConstantBufferUploader<Struct::CBSpritePass>
	{
	public:

		 SpritePassConstantBufferUploader()          = default;
		~SpritePassConstantBufferUploader() override = default;

		FWK_DEFINE_TYPE_INFO(SpritePassConstantBufferUploader, ConstantBufferUploaderBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::ConstantBufferSharedFactory, FWK::Graphics::SpritePassConstantBufferUploader)