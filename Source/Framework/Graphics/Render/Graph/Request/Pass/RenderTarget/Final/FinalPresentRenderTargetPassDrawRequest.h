#pragma once

namespace FWK::Graphics
{
	class FinalPresentRenderTargetPassDrawRequest final : public DrawRequestRenderTargetPassBase<Struct::CBFinalPresentRenderTargetPass, Enum::RenderGraphResourceType::SceneColor>
	{
	public:

		 FinalPresentRenderTargetPassDrawRequest()			= default;
		~FinalPresentRenderTargetPassDrawRequest() override = default;

		bool SetupPassConstantBuffer(const RootSignature& a_rootSignature, const DirectCommandList& a_directCommandList, const FrameResource& a_frameResource) override;

		FWK_DEFINE_TYPE_INFO(FinalPresentRenderTargetPassDrawRequest, DrawRequestPassBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DrawRequestPassSharedFactory, FWK::Graphics::FinalPresentRenderTargetPassDrawRequest)