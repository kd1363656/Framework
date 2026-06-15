#pragma once

namespace FWK::Graphics
{
	class FinalColorRenderTargetPassConstantBufferUploader final : public ConstantBufferUploader<Struct::CBFinalColorRenderTargetPass>
	{
	public:

		 FinalColorRenderTargetPassConstantBufferUploader()          = default;
		~FinalColorRenderTargetPassConstantBufferUploader() override = default;

		FWK_DEFINE_TYPE_INFO(FinalColorRenderTargetPassConstantBufferUploader, ConstantBufferUploaderBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::ConstantBufferSharedFactory, FWK::Graphics::FinalColorRenderTargetPassConstantBufferUploader)