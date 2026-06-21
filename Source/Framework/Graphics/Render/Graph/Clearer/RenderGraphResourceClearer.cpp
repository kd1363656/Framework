#include "RenderGraphResourceClearer.h"

void FWK::Graphics::RenderGraphResourceClearer::ClearCurrentFramePassTextureList(const ResourceContext& a_resourceContext, const Renderer& a_renderer)
{
	const auto& l_currentFrameResource = a_renderer.GetREFCurrentFrameResource().lock();

	FWK_ASSERT_RETURN_IF_FAILED(!l_currentFrameResource, "現在のFrameResourceが無効のため、PassTextureListのClearに失敗しました。");

	const auto& l_renderGraphFrameResource = l_currentFrameResource->GetREFRenderGraphFrameResource();

	// 毎フレームクリアすべきレンダーターゲットテクスチャとデプスステンシルテクスチャのクリア処理を行う
	ClearCurrentFrameRenderTargetPassTextureList(a_resourceContext, a_renderer, l_renderGraphFrameResource);
	ClearCurrentFrameDepthStencilPassTextureList(a_resourceContext, a_renderer, l_renderGraphFrameResource);
}

void FWK::Graphics::RenderGraphResourceClearer::ClearCurrentFrameRenderTargetPassTextureList(const ResourceContext& a_resourceContext, const Renderer& a_renderer, const RenderGraphFrameResource& a_renderGraphFrameResource) const
{
	for (const auto& l_renderTargetPassTexture : a_renderGraphFrameResource.GetREFRenderTargetPassTextureList())
	{
		FWK_ASSERT_RETURN_IF_FAILED(!l_renderTargetPassTexture,																  "RenderTargetPassTextureが無効のため、RenderTargetPassTextureのClearに失敗しました。");
		FWK_ASSERT_RETURN_IF_FAILED(!ClearRenderTargetPassTexture(a_resourceContext, a_renderer, *l_renderTargetPassTexture), "RenderTargetPassTextureのClearに失敗しました。");
	}
}
void FWK::Graphics::RenderGraphResourceClearer::ClearCurrentFrameDepthStencilPassTextureList(const ResourceContext& a_resourceContext, const Renderer& a_renderer, const RenderGraphFrameResource& a_renderGraphFrameResource) const
{
}

bool FWK::Graphics::RenderGraphResourceClearer::ClearRenderTargetPassTexture(const ResourceContext& a_resourceContext, const Renderer& a_renderer, RenderTargetPassTexture& a_renderTargetPassTexture) const
{
		  auto& l_renderTargetTexture = a_renderTargetPassTexture.GetMutableREFRenderTargetTexture();
	const auto& l_gpuResource		  = l_renderTargetTexture.GetREFGPUResource					  ();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_gpuResource.m_resource,																  "RenderTargetPassTextureのGPUResourceが無効のため、Clear処理に失敗しました。",    false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_renderTargetTexture.GetVALRTVDescriptorIndex() == Constant::k_invalidDescriptorIndex, "RenderTargetPassTextureのRTVDescriptorIndexが無効のため、Clearに失敗しました。", false);

	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList	   ();
	const auto& l_rtvDescriptorPool = a_resourceContext.GetREFRTVDescriptorPool();

	// RenderTargetをClearするにはD3D12_RESOURCE_STATE_RENDER_TARGET状態である必要がある。
	// 現在の状態が違う場合だけResourceBarrierを張る。
	
	return true;
}
bool FWK::Graphics::RenderGraphResourceClearer::ClearDepthStencilPassTexture(const ResourceContext& a_resourceContext, const Renderer& a_renderer, DepthStencilPassTexture& a_depthStencilPassTexture) const
{
	return true;
}