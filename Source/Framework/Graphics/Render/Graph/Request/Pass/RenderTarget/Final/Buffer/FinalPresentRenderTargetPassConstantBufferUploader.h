#pragma once

namespace FWK::Graphics
{
	class FinalPresentRenderTargetPassConstantBufferUploader final : public ConstantBufferUploader<Struct::CBFinalPresentRenderTargetPass>
	{
	public:

		 FinalPresentRenderTargetPassConstantBufferUploader()		   = default;
		~FinalPresentRenderTargetPassConstantBufferUploader() override = default;

		FWK_DEFINE_TYPE_INFO(FinalPresentRenderTargetPassConstantBufferUploader, ConstantBufferUploaderBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::ConstantBufferSharedFactory, FWK::Graphics::FinalPresentRenderTargetPassConstantBufferUploader)