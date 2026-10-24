#include "SkeletalAnimationModelGPUSkinnedCascadeShadowPerObjectDrawRequest.h"

void FWK::Graphics::SkeletalAnimationModelGPUSkinnedCascadeShadowPerObjectDrawRequest::BeginFrame()
{
    m_skeletalAnimationModelGPUSkinnedPerObjectDrawRequest.BeginFrame();
}

void FWK::Graphics::SkeletalAnimationModelGPUSkinnedCascadeShadowPerObjectDrawRequest::SetupPerObjectConstantBuffer(const Renderer& a_renderer, const RootSignature& a_rootSignature, const FrameResource& a_frameResource)
{
    m_skeletalAnimationModelGPUSkinnedPerObjectDrawRequest.SetupPerObjectConstantBuffer(a_renderer, a_rootSignature, a_frameResource);
}

void FWK::Graphics::SkeletalAnimationModelGPUSkinnedCascadeShadowPerObjectDrawRequest::AddDrawRequest(const std::shared_ptr<Struct::SkeletalAnimationModelGPUSkinnedDrawRequestData>& a_drawRequestData)
{
    FWK_ASSERT_RETURN_IF(!a_drawRequestData, "DrawRequestDataが無効なため、描画申請を追加できません。");

    m_skeletalAnimationModelGPUSkinnedPerObjectDrawRequest.AddDrawRequest(a_drawRequestData);
}

bool FWK::Graphics::SkeletalAnimationModelGPUSkinnedCascadeShadowPerObjectDrawRequest::DispatchGPUCulling(Renderer& a_renderer, const std::size_t a_cascadeIndex, const Struct::ModelGPUDrivenCullingView& a_cullingView)
{
    return m_skeletalAnimationModelGPUSkinnedPerObjectDrawRequest.DispatchGPUCulling(a_renderer, a_cascadeIndex, a_cullingView);
}