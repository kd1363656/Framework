#include "RenderGraph.h"

void FWK::Graphics::RenderGraph::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}
void FWK::Graphics::RenderGraph::BeginFrame(const ResourceContext& a_resourceContext, const FrameResource& a_frameResource, const Renderer& a_renderer)
{
	// 使用、ポインタがnullのパスの削除
	RemoveExpiredPassList();
	
	// 共通定数バッファのパラメータを更新
	for (const auto& l_drawRequestPass : m_drawRequestPassList)
	{
		FWK_ASSERT_RETURN_IF_FAILED(!l_drawRequestPass, "DrawRequestPassが無効のため、BeginFrame処理に失敗しました。");

		l_drawRequestPass->BeginFrame();
	}
	
	ClearBackBuffer(a_resourceContext, a_renderer);

	ExecutePassList(a_resourceContext, a_frameResource, a_renderer);
}

void FWK::Graphics::RenderGraph::EndFrame(const Renderer& a_renderer) const
{
	const auto& l_swapChain			= a_renderer.GetREFSwapChain		();
	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();
	
	const auto  l_backBufferIndex = l_swapChain.FetchVALCurrentBackBufferIndex();
	const auto& l_backBufferList  = l_swapChain.GetREFBackBufferList		  ();

	FWK_ASSERT_RETURN_IF_FAILED(l_backBufferList.empty(),										 "BackBufferListが空のため、BackBufferのClearに失敗しました。");
	FWK_ASSERT_RETURN_IF_FAILED(l_backBufferIndex >= static_cast<UINT>(l_backBufferList.size()), "BackBufferIndexが範囲外のため、BackBufferのClearに失敗しました。");

	const auto& l_backBuffer = l_backBufferList[l_backBufferIndex];

	FWK_ASSERT_RETURN_IF_FAILED(l_backBuffer.m_rtvDescriptorIndex == Constant::k_invalidDescriptorIndex, "BackBufferのRTVDescriptorIndexが無効のため、BackBufferのClearに失敗しました。");

	// BackBufferをRENDERTARGET -> PRESENTへ遷移
	TransitionBackBufferResource(l_directCommandList,
								 D3D12_RESOURCE_STATE_RENDER_TARGET, 
								 D3D12_RESOURCE_STATE_PRESENT, 
								 l_backBuffer);
}

nlohmann::json FWK::Graphics::RenderGraph::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

void FWK::Graphics::RenderGraph::AddPass(std::unique_ptr<RenderGraphPassBase>&& a_pass)
{
	FWK_ASSERT_RETURN_IF_FAILED(!a_pass, "RenderGraphPassが無効のため、PassListへの登録処理に失敗しました。");

	m_passList.emplace_back(std::move(a_pass));
}

void FWK::Graphics::RenderGraph::AddDrawRequestPass(const std::shared_ptr<DrawRequestPassBase>& a_drawRequestPass)
{
	FWK_ASSERT_RETURN_IF_FAILED(!a_drawRequestPass, "DrawRequestPassが無効のため、DrawRequestPassListへの登録に失敗しました。");

	const auto l_staticTypeID = a_drawRequestPass->GetREFRuntimeTypeINFO().k_staticTypeID;

	FWK_ASSERT_RETURN_IF_FAILED(m_drawRequestPassMap.contains(l_staticTypeID), "同じ型のDrawRequestPassを二重登録しようとしており、DrawRequestPassMapへの登録に失敗しました。");

	m_drawRequestPassList.emplace_back(a_drawRequestPass);
	m_drawRequestPassMap.try_emplace  (l_staticTypeID, a_drawRequestPass);
}

void FWK::Graphics::RenderGraph::ExecutePassList(const ResourceContext& a_resourceContext, const FrameResource& a_frameResource, const Renderer& a_renderer) const
{
	for (const auto& l_pass : m_passList)
	{
		if (!l_pass) { continue; }
		
		l_pass->Execute(a_resourceContext, a_frameResource, a_renderer);
	}
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

void FWK::Graphics::RenderGraph::RemoveExpiredPassList()
{
	std::size_t l_index = 0ULL;

	while (l_index < m_passList.size())
	{
		if (m_passList[l_index]) 
		{
			++l_index;
			continue;
		}

		std::swap          (m_passList[l_index], m_passList.back());
		m_passList.pop_back();
	}
}