#pragma once

#pragma once

namespace FWK::Graphics
{
	class StaticModelPerObjectDynamicConstantBufferUploader final : public DynamicConstantBufferUploaderBase<Struct::CBStaticModelPerObject>
	{
	public:

		 StaticModelPerObjectDynamicConstantBufferUploader()          = default;
		~StaticModelPerObjectDynamicConstantBufferUploader() override = default;

		FWK_DEFINE_TYPE_INFO(StaticModelPerObjectDynamicConstantBufferUploader, DynamicBufferUploaderBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DynamicBufferSharedFactory, FWK::Graphics::StaticModelPerObjectDynamicConstantBufferUploader)