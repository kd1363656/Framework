#pragma once

#pragma once

namespace FWK::Graphics
{
	class StaticModelPerObjectConstantBufferUploader final : public ConstantBufferUploader<Struct::CBStaticModelPerObject>
	{
	public:

		 StaticModelPerObjectConstantBufferUploader()          = default;
		~StaticModelPerObjectConstantBufferUploader() override = default;

		FWK_DEFINE_TYPE_INFO(StaticModelPerObjectConstantBufferUploader, ConstantBufferUploaderBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::ConstantBufferSharedFactory, FWK::Graphics::StaticModelPerObjectConstantBufferUploader)