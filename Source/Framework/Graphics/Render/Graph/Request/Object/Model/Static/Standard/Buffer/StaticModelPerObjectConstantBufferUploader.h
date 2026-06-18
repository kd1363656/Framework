#pragma once

#pragma once

namespace FWK::Graphics
{
	class StaticModelPerObjectConstantBufferUploader final : public ConstantBufferUploader<Struct::CBStaticModelPerObject>
	{
	public:

		 StaticModelPerObjectConstantBufferUploader()          = default;
		~StaticModelPerObjectConstantBufferUploader() override = default;

	private:

		static constexpr UINT64 k_bufferTypeSize = sizeof(Struct::CBStaticModelPerObject);

		FWK_DEFINE_TYPE_INFO(ModelPerObjectConstantBufferUploader, ConstantBufferUploaderBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::ConstantBufferSharedFactory, FWK::Graphics::StaticModelPerObjectConstantBufferUploader)