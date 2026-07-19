#pragma once

namespace FWK::Graphics
{
	class SpriteScreenPassDynamicConstantBufferUploader final : public DynamicConstantBufferFixedWritePositionUploaderBase<Struct::CBSpritePass>
	{
	public:

		 SpriteScreenPassDynamicConstantBufferUploader()          = default;
		~SpriteScreenPassDynamicConstantBufferUploader() override = default;

		FWK_DEFINE_TYPE_INFO(SpriteScreenPassDynamicConstantBufferUploader, DynamicBufferUploaderBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DynamicBufferSharedFactory, FWK::Graphics::SpriteScreenPassDynamicConstantBufferUploader)