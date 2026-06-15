#pragma once

namespace FWK::Graphics
{
	class FinalColorRenderTargetPassDrawRequest final : public DrawRequestRenderTargetPassBase<Struct::CBFinalColorRenderTargetPass, Enum::RenderGraphResourceType::SceneColor>
	{
	public:

		 FinalColorRenderTargetPassDrawRequest()          = default;
		~FinalColorRenderTargetPassDrawRequest() override = default;

		bool SetupPassConstantBuffer(const RootSignature& a_rootSignature, const DirectCommandList& a_directCommandList, const FrameResource& a_frameResource) override;

		FWK_DEFINE_TYPE_INFO(FinalColorRenderTargetPassDrawRequest, DrawRequestPassBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DrawRequestPassSharedFactory, FWK::Graphics::FinalColorRenderTargetPassDrawRequest)