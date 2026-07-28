#include "StaticModelShadowPerObjectDrawRequest.h"

void FWK::Graphics::StaticModelShadowPerObjectDrawRequest::BeginFrame()
{
	m_staticModelPerObjectDrawRequest.BeginFrame();
}

void FWK::Graphics::StaticModelShadowPerObjectDrawRequest::SetupPerObjectConstantBuffer(const Renderer& a_renderer, const RootSignature& a_rootSignature, const FrameResource& a_frameResource)
{
	m_staticModelPerObjectDrawRequest.SetupPerObjectConstantBuffer(a_renderer, a_rootSignature, a_frameResource);
}

void FWK::Graphics::StaticModelShadowPerObjectDrawRequest::AddDrawRequest(const std::shared_ptr<Struct::StaticModelPerObjectDrawRequestData>&a_drawRequestData)
{
	FWK_ASSERT_RETURN_IF(!a_drawRequestData, "DrawRequestDataが無効のため、描画申請の追加が出来ませんでした。");

	m_staticModelPerObjectDrawRequest.AddDrawRequest(a_drawRequestData);
}