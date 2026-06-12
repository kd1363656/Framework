#pragma once

namespace FWK::Graphics
{
	class SpriteScreenPassConstantBufferUploader final : public ConstantBufferUploader<Struct::CBSpritePass>
	{
	public:

		 SpriteScreenPassConstantBufferUploader()          = default;
		~SpriteScreenPassConstantBufferUploader() override = default;

		FWK_DEFINE_TYPE_INFO(SpriteScreenPassConstantBufferUploader, ConstantBufferUploaderBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::ConstantBufferSharedFactory, FWK::Graphics::SpriteScreenPassConstantBufferUploader)