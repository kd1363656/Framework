#pragma once

namespace FWK::Graphics
{
	class StaticModelShadowPerObjectDrawRequest final : public DrawRequestPerObjectBase
	{
	public:

		 StaticModelShadowPerObjectDrawRequest()          = default;
		~StaticModelShadowPerObjectDrawRequest() override = default;

		void BeginFrame() override;

		void SetupPerObjectConstantBuffer(const Renderer& a_renderer, const RootSignature& a_rootSignature, const FrameResource& a_frameResource) override;

		void AddDrawRequest(const std::shared_ptr<Struct::StaticModelPerObjectDrawRequestData>& a_drawRequestData);
		
	private:

		StaticModelPerObjectDrawRequestBase m_staticModelPerObjectDrawRequest = {};
		
		FWK_DEFINE_TYPE_INFO(StaticModelShadowPerObjectDrawRequest, DrawRequestPerObjectBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DrawRequestPerObjectSharedFactory, FWK::Graphics::StaticModelShadowPerObjectDrawRequest)