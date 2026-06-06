#include "RenderGraph.h"

void FWK::Graphics::RenderGraph::BeginFrame(const ResourceContext& a_resourceContext, const Renderer& a_renderer) const
{
	ClearBackBuffer(a_resourceContext, a_renderer);
}
void FWK::Graphics::RenderGraph::EndFrame(const Renderer& a_renderer) const
{
	const auto& l_swapChain			= a_renderer.GetREFSwapChain		       ();
	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList       ();
	
	const auto  l_backBufferIndex = l_swapChain.FetchVALCurrentBackBufferIndex();
	const auto& l_backBufferList  = l_swapChain.GetREFBackBufferList		  ();

	FWK_ASSERT_RETURN_IF_FAILED(l_backBufferList.empty(),										 "BackBufferListが空のため、BackBufferのClearに失敗しました。");
	FWK_ASSERT_RETURN_IF_FAILED(l_backBufferIndex >= static_cast<UINT>(l_backBufferList.size()), "BackBufferIndexが範囲外のため、BackBufferのClearに失敗しました。");

	const auto& l_backBuffer = l_backBufferList[l_backBufferIndex];

	FWK_ASSERT_RETURN_IF_FAILED(l_backBuffer.m_rtvDescriptorIndex == Constant::k_invalidDescriptorIndex, "BackBufferのRTVDescriptorIndexが無効のため、BackBufferのClearに失敗しました。");

	// BackBufferをRENDERTARGET -> PRESENT
	TransitionBackBufferResource(l_directCommandList,
								 D3D12_RESOURCE_STATE_RENDER_TARGET, 
								 D3D12_RESOURCE_STATE_PRESENT, 
								 l_backBuffer);
}

void FWK::Graphics::RenderGraph::ClearBackBuffer(const ResourceContext& a_resourceContext, const Renderer& a_renderer) const
{
	const auto& l_swapChain			= a_renderer.GetREFSwapChain		       ();
	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList       ();
	const auto& l_rtvDescriptorPool = a_resourceContext.GetREFRTVDescriptorPool();

	const auto  l_backBufferIndex = l_swapChain.FetchVALCurrentBackBufferIndex();
	const auto& l_backBufferList  = l_swapChain.GetREFBackBufferList		  ();

	FWK_ASSERT_RETURN_IF_FAILED(l_backBufferList.empty(), "BackBufferListが空のため、BackBufferのClearに失敗しました。");
	FWK_ASSERT_RETURN_IF_FAILED(l_backBufferIndex >= static_cast<UINT>(l_backBufferList.size()), "BackBufferIndexが範囲外のため、BackBufferのClearに失敗しました。");

	const auto& l_backBuffer = l_backBufferList[l_backBufferIndex];

	FWK_ASSERT_RETURN_IF_FAILED(l_backBuffer.m_rtvDescriptorIndex == Constant::k_invalidDescriptorIndex, "BackBufferのRTVDescriptorIndexが無効のため、BackBufferのClearに失敗しました。");

	// BackBufferをPRESENT -> RENDERTARGET
	TransitionBackBufferResource(l_directCommandList,
								 D3D12_RESOURCE_STATE_PRESENT,
								 D3D12_RESOURCE_STATE_RENDER_TARGET, 
								 l_backBuffer);

	// このバックバッファを描画先として設定する
	l_directCommandList.SetupRenderTarget(l_rtvDescriptorPool, l_backBuffer.m_rtvDescriptorIndex);

	// 描画先に設定したBackBufferを指定色でClearする。
	l_directCommandList.ClearRenderTarget(l_rtvDescriptorPool, l_backBuffer.m_rtvDescriptorIndex, k_backBufferClearColor);
}

void FWK::Graphics::RenderGraph::TransitionBackBufferResource(const DirectCommandList&	  a_directCommandList, 
															  const D3D12_RESOURCE_STATES a_beforeState, 
															  const D3D12_RESOURCE_STATES a_afterState,
															  const	Struct::BackBuffer&	  a_backBuffer) const
{
	FWK_ASSERT_RETURN_IF_FAILED(!a_backBuffer.m_backBufferResource, "バックバッファリソースが無効になっており、バックバッファリソースの状態遷移に失敗しました。");

	a_directCommandList.TransitionResourceBarrier(a_backBuffer.m_backBufferResource, a_beforeState, a_afterState);
}