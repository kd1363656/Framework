#pragma once

namespace FWK::Graphics
{
	class SpritePerObjectConstantBufferUploader final : public ConstantBufferUploader<Struct::CBSpritePerObject>
	{
	public:

		 SpritePerObjectConstantBufferUploader()          = default;
		~SpritePerObjectConstantBufferUploader() override = default;

		FWK_DEFINE_TYPE_INFO(SpritePerObjectConstantBufferUploader, ConstantBufferUploaderBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::ConstantBufferSharedFactory, FWK::Graphics::SpritePerObjectConstantBufferUploader)