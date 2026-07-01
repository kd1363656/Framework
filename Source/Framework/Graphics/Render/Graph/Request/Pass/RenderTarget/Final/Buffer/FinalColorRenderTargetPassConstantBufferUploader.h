#pragma once

namespace FWK::Graphics
{
	class FinalColorRenderTargetPassDynamicConstantBufferUploader final : public DynamicConstantBufferUploaderBase<Struct::CBFinalColorRenderTargetPass>
	{
	public:

		 FinalColorRenderTargetPassDynamicConstantBufferUploader()          = default;
		~FinalColorRenderTargetPassDynamicConstantBufferUploader() override = default;

		FWK_DEFINE_TYPE_INFO(FinalColorRenderTargetPassDynamicConstantBufferUploader, DynamicBufferUploaderBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DynamicBufferSharedFactory, FWK::Graphics::FinalColorRenderTargetPassDynamicConstantBufferUploader)