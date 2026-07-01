#pragma once

namespace FWK::Graphics
{
	class SpriteScreenPerObjectDynamicConstantBufferUploader final : public DynamicConstantBufferUploaderBase<Struct::CBSpritePerObject>
	{
	public:

		 SpriteScreenPerObjectDynamicConstantBufferUploader()          = default;
		~SpriteScreenPerObjectDynamicConstantBufferUploader() override = default;

		FWK_DEFINE_TYPE_INFO(SpriteScreenPerObjectDynamicConstantBufferUploader, DynamicBufferUploaderBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DynamicBufferSharedFactory, FWK::Graphics::SpriteScreenPerObjectDynamicConstantBufferUploader)