#pragma once

namespace FWK::Graphics
{
	class SpriteScreenPerObjectConstantBufferUploader final : public ConstantBufferUploader<Struct::CBSpritePerObject>
	{
	public:

		 SpriteScreenPerObjectConstantBufferUploader()          = default;
		~SpriteScreenPerObjectConstantBufferUploader() override = default;

		FWK_DEFINE_TYPE_INFO(SpriteScreenPerObjectConstantBufferUploader, ConstantBufferUploaderBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::ConstantBufferSharedFactory, FWK::Graphics::SpriteScreenPerObjectConstantBufferUploader)