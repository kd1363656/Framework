#pragma once

namespace FWK::Graphics
{
	class FinalPresentRenderTargetPassDynamicConstantBufferUploader final : public DynamicConstantBufferUploaderBase<Struct::CBFinalPresentRenderTargetPass>
	{
	public:

		 FinalPresentRenderTargetPassDynamicConstantBufferUploader()          = default;
		~FinalPresentRenderTargetPassDynamicConstantBufferUploader() override = default;

		FWK_DEFINE_TYPE_INFO(FinalPresentRenderTargetPassDynamicConstantBufferUploader, DynamicBufferUploaderBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DynamicBufferSharedFactory, FWK::Graphics::FinalPresentRenderTargetPassDynamicConstantBufferUploader)