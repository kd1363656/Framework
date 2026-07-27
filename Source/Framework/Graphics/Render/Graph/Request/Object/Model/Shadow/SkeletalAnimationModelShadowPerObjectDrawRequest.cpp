#include "ModelShadowPerObjectDrawRequest.h"

void FWK::Graphics::ModelShadowPerObjectDrawRequest::BeginFrame()
{
	m_staticModelStandardPerObjectDrawRequest.BeginFrame           ();
	m_skeletalAnimationModelStandardPerObjectDrawRequest.BeginFrame();
}

void FWK::Graphics::ModelShadowPerObjectDrawRequest::SetupPerObjectConstantBuffer(const Renderer& a_renderer, const RootSignature& a_rootSignature, const FrameResource& a_frameResource)
{
	m_staticModelStandardPerObjectDrawRequest.SetupPerObjectConstantBuffer           (a_renderer, a_rootSignature, a_frameResource);
	m_skeletalAnimationModelStandardPerObjectDrawRequest.SetupPerObjectConstantBuffer(a_renderer, a_rootSignature, a_frameResource);
}

void FWK::Graphics::ModelShadowPerObjectDrawRequest::AddDrawRequest(const std::shared_ptr<Struct::StaticModelStandardPerObjectDrawRequestData>&a_drawRequestData)
{
	FWK_ASSERT_RETURN_IF(!a_drawRequestData, "DrawRequestDataが無効のため、描画申請の追加が出来ませんでした。");

	m_staticModelStandardPerObjectDrawRequest.AddDrawRequest(a_drawRequestData);
}
void FWK::Graphics::ModelShadowPerObjectDrawRequest::AddDrawRequest(const std::shared_ptr<Struct::SkeletalAnimationModelStandardPerObjectDrawRequestData>& a_drawRequestData)
{
	FWK_ASSERT_RETURN_IF(!a_drawRequestData, "DrawRequestDataが無効なため、SkeletalAnimationModelの描画申請を追加できません。");

	m_skeletalAnimationModelStandardPerObjectDrawRequest.AddDrawRequest(a_drawRequestData);
}