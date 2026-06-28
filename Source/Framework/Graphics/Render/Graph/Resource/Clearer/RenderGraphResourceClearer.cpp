#include "RenderGraphResourceClearer.h"

void FWK::Graphics::RenderGraphResourceClearer::ClearCurrentFramePassTextureList(const ResourceContext& a_resourceContext, const Renderer& a_renderer) const
{
	const auto& l_currentFrameResource = a_renderer.GetREFCurrentFrameResource().lock();

	FWK_ASSERT_RETURN_IF(!l_currentFrameResource, "現在のFrameResourceが無効のため、PassTextureListのClearに失敗しました。");

	const auto& l_renderGraphFrameResource = l_currentFrameResource->GetREFRenderGraphFrameResource();

	// 毎フレームクリアすべきレンダーターゲットテクスチャとデプスステンシルテクスチャのクリア処理を行う
	ClearCurrentFrameRenderTargetPassTextureList(a_resourceContext, a_renderer, l_renderGraphFrameResource);
	ClearCurrentFrameDepthStencilPassTextureList(a_resourceContext, a_renderer, l_renderGraphFrameResource);
}

void FWK::Graphics::RenderGraphResourceClearer::ClearCurrentFrameRenderTargetPassTextureList(const ResourceContext& a_resourceContext, const Renderer& a_renderer, const RenderGraphFrameResource& a_renderGraphFrameResource) const
{
	for (const auto& l_renderTargetPassTexture : a_renderGraphFrameResource.GetREFRenderTargetPassTextureList())
	{
		FWK_ASSERT_RETURN_IF(!l_renderTargetPassTexture,																   "RenderTargetPassTextureが無効のため、RenderTargetPassTextureのClearに失敗しました。");
		FWK_ASSERT_RETURN_IF(!ClearRenderTargetPassTexture(a_resourceContext, a_renderer, *l_renderTargetPassTexture), "RenderTargetPassTextureのClearに失敗しました。");
	}
}
void FWK::Graphics::RenderGraphResourceClearer::ClearCurrentFrameDepthStencilPassTextureList(const ResourceContext& a_resourceContext, const Renderer& a_renderer, const RenderGraphFrameResource& a_renderGraphFrameResource) const
{
	for (const auto& l_depthStencilPassTexture : a_renderGraphFrameResource.GetREFDepthStencilPassTextureList())
	{
		FWK_ASSERT_RETURN_IF(!l_depthStencilPassTexture,																   "DepthStencilPassTextureが無効のため、DepthStencilPassTextureのClearに失敗しました。");
		FWK_ASSERT_RETURN_IF(!ClearDepthStencilPassTexture(a_resourceContext, a_renderer, *l_depthStencilPassTexture), "DepthStencilPassTextureのClearに失敗しました。");
	}
}

bool FWK::Graphics::RenderGraphResourceClearer::ClearRenderTargetPassTexture(const ResourceContext& a_resourceContext, const Renderer& a_renderer, const RenderTargetPassTexture& a_renderTargetPassTexture) const
{
	const auto& l_renderTargetTexture = a_renderTargetPassTexture.GetREFRenderTargetTexture();
	const auto& l_gpuResource		  = l_renderTargetTexture.GetREFGPUResource			   ();

	FWK_ASSERT_RETURN_VALUE_IF(!l_gpuResource.m_resource,															   "RenderTargetPassTextureのGPUResourceが無効のため、Clear処理に失敗しました。",    false);
	FWK_ASSERT_RETURN_VALUE_IF(l_renderTargetTexture.GetVALRTVDescriptorIndex() == Constant::k_invalidDescriptorIndex, "RenderTargetPassTextureのRTVDescriptorIndexが無効のため、Clearに失敗しました。", false);

	// RenderGraph側でリソースの遷移漏れがあればassert
	FWK_ASSERT_RETURN_VALUE_IF(l_renderTargetTexture.GetVALCurrentResourceState() != D3D12_RESOURCE_STATE_RENDER_TARGET, "RenderTargetPassTextureがD3D12_RESOURCE_STATE_RENDER_TARGETではない状態でClearしようとしています。beforUsage/afterUsageの指定、またはRenderGraphの自動遷移を確認してください。", false);
	
	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList	   ();
	const auto& l_rtvDescriptorPool = a_resourceContext.GetREFRTVDescriptorPool();

	// レンダーターゲットテクスチャの指定職でバックバッファをクリア
	l_directCommandList.ClearRenderTarget(l_rtvDescriptorPool, l_renderTargetTexture.GetVALRTVDescriptorIndex(), a_renderTargetPassTexture.GetREFClearColor());

	return true;
}
bool FWK::Graphics::RenderGraphResourceClearer::ClearDepthStencilPassTexture(const ResourceContext& a_resourceContext, const Renderer& a_renderer, const DepthStencilPassTexture& a_depthStencilPassTexture) const
{
	const auto& l_depthStencilTexture = a_depthStencilPassTexture.GetREFDepthStencilTexture();
	const auto& l_gpuResource		  = l_depthStencilTexture.GetREFGPUResource			   ();

	FWK_ASSERT_RETURN_VALUE_IF(!l_gpuResource.m_resource,															   "DepthStencilPassTextureのGPUResourceが無効のため、Clear処理に失敗しました。",    false);
	FWK_ASSERT_RETURN_VALUE_IF(l_depthStencilTexture.GetVALDSVDescriptorIndex() == Constant::k_invalidDescriptorIndex, "DepthStencilPassTextureのDSVDescriptorIndexが無効のため、Clearに失敗しました。", false);

	// RenderGraph側でリソースの遷移漏れがあればassert
	FWK_ASSERT_RETURN_VALUE_IF(l_depthStencilTexture.GetVALCurrentResourceState() != D3D12_RESOURCE_STATE_DEPTH_WRITE, "DepthStencilPassTextureがD3D12_RESOURCE_STATE_DEPTH_WRITEではない状態でClearしようとしています。beforUsage/afterUsageの指定、またはRenderGraphの自動遷移を確認してください。", false);
	
	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList	   ();
	const auto& l_dsvDescriptorPool = a_resourceContext.GetREFDSVDescriptorPool();

	// レンダーターゲットテクスチャの指定職でバックバッファをクリア
	l_directCommandList.ClearDepthStencil(l_dsvDescriptorPool,
										  a_depthStencilPassTexture.GetVALDepthClearValue(),
										  l_depthStencilTexture.GetVALDSVDescriptorIndex(), 
										  a_depthStencilPassTexture.GetVALStencilClearValue());

	return true;
}