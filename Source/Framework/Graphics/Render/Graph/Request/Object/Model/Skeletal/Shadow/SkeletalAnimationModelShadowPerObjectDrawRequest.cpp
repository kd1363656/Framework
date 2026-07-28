#include "SkeletalAnimationModelShadowPerObjectDrawRequest.h"

void FWK::Graphics::SkeletalAnimationModelShadowPerObjectDrawRequest::BeginFrame()
{
	m_skeletalAnimationModelPerObjectDrawRequest.BeginFrame();
}

void FWK::Graphics::SkeletalAnimationModelShadowPerObjectDrawRequest::SetupPerObjectConstantBuffer(const Renderer& a_renderer, const RootSignature& a_rootSignature, const FrameResource& a_frameResource)
{
	m_skeletalAnimationModelPerObjectDrawRequest.SetupPerObjectConstantBuffer(a_renderer, a_rootSignature, a_frameResource);
}

void FWK::Graphics::SkeletalAnimationModelShadowPerObjectDrawRequest::AddDrawRequest(const std::shared_ptr<Struct::SkeletalAnimationModelPerObjectDrawRequestData>&a_drawRequestData)
{
	FWK_ASSERT_RETURN_IF(!a_drawRequestData, "DrawRequestDataが無効なため、SkeletalAnimationModelの描画申請を追加できません。");

	m_skeletalAnimationModelPerObjectDrawRequest.AddDrawRequest(a_drawRequestData);
}