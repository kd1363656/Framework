#pragma once

namespace FWK::Graphics
{
	class CascadeShadowMapPassDrawRequest final : public DrawRequestPassBase
	{
	public:

		 CascadeShadowMapPassDrawRequest()		    = default;
		~CascadeShadowMapPassDrawRequest() override = default;

		bool SetupPassConstantBuffer(const RootSignature& a_rootSignature, const DirectCommandList& a_directCommandList, const FrameResource& a_frameResource) override;

		FWK_DEFINE_TYPE_INFO(CascadeShadowMapPassDrawRequest, DrawRequestPassBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DrawRequestPassSharedFactory, FWK::Graphics::CascadeShadowMapPassDrawRequest)