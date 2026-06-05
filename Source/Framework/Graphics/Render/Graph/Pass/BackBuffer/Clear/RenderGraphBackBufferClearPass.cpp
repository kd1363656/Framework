#include "RenderGraphBackBufferClearPass.h"

FWK::Graphics::RenderGraphBackBufferClearPass::RenderGraphBackBufferClearPass() : 
	RenderGraphPassBase(Enum::RenderGraphPassType::BackBufferClear)
{
	// このPassはBackBufferをRenderTargetとしてClearする。
	// 実際のPRESENT -> RENDER_TARGETの状態遷移はRenderGraph側で自動実行する。
	AddResourceAccess(Enum::RenderGraphResourceType::BackBuffer, Enum::RenderGraphResourceAccessType::ClearWrite);
}
FWK::Graphics::RenderGraphBackBufferClearPass::~RenderGraphBackBufferClearPass() = default;

void FWK::Graphics::RenderGraphBackBufferClearPass::Execute(const ResourceContext& a_resourceContext, Renderer& a_renderer)
{
	const auto& l_rtvDescriptorPool = a_resourceContext.GetREFRTVDescriptorPool();

	const auto& l_swapChain			= a_renderer.GetREFSwapChain	    ();
	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();

	const auto  l_backBufferIndex = l_swapChain.FetchVALCurrentBackBufferIndex();
	const auto& l_backBufferList  = l_swapChain.GetREFBackBufferList		  ();

	FWK_ASSERT_RETURN_IF_FAILED(l_backBufferIndex >= l_backBufferList.size(), "現在のBackBufferIndexがBackBufferListの範囲外になっており、BackBufferClearPassの実行に失敗しました。");

	const auto& l_backBuffer = l_backBufferList[l_backBufferIndex];

	FWK_ASSERT_RETURN_IF_FAILED(!l_backBuffer.m_backBufferResource,										 "BackBufferResourceが無効のため、BackBufferClearPassの実行に失敗しました。");
	FWK_ASSERT_RETURN_IF_FAILED(l_backBuffer.m_rtvDescriptorIndex == Constant::k_invalidDescriptorIndex, "BackBufferのRTVDescriptorIndexが無効のため、、BackBufferClearPassの実行に失敗しました。");

	// この時点でBackBufferはRenderGraphにより、D3D12_RESOURCE_STATE_RENDER_TARGETに遷移済み
	l_directCommandList.SetupRenderTarget(l_rtvDescriptorPool, l_backBuffer.m_rtvDescriptorIndex);
	l_directCommandList.ClearRenderTarget(l_rtvDescriptorPool, l_backBuffer.m_rtvDescriptorIndex, k_backBufferClearColor);
}