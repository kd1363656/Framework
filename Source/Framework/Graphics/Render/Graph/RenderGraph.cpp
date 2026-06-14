#include "RenderGraph.h"

void FWK::Graphics::RenderGraph::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}
void FWK::Graphics::RenderGraph::BeginFrame(const ResourceContext& a_resourceContext, Renderer& a_renderer) 
{
	// 使用、ポインタがnullのパスの削除
	RemoveExpiredPassList();
	
	for (const auto& l_drawRequestPerObject : m_drawRequestPerObjectList)
	{
		FWK_ASSERT_RETURN_IF_FAILED       (!l_drawRequestPerObject, "DrawRequestPerObjectが無効のため、BeginFrame処理に失敗しました。");
		l_drawRequestPerObject->BeginFrame();
	}

	ClearBackBuffer(a_resourceContext, a_renderer);
}

void FWK::Graphics::RenderGraph::Execute(const ResourceContext& a_resourceContext, Renderer& a_renderer) const
{
	const auto& l_srvDescriptorPool = a_resourceContext.GetREFSRVDescriptorPool();
	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList	   ();

	// SRVディスクリプタヒープをセット
	l_directCommandList.SetupDescriptorHeap(l_srvDescriptorPool);

	for (const auto& l_pass : m_passList)
	{
		if (!l_pass) { continue; }
		
		// Passが宣言しているResourceAccessListを見て、実行直前に必要な状態へ自動遷移する
		TransitionPassResource(*l_pass, a_renderer);

		l_pass->Execute(a_resourceContext, a_renderer);
	}
}

void FWK::Graphics::RenderGraph::EndFrame(Renderer& a_renderer) const
{
		 auto& l_swapChain			= a_renderer.GetMutableREFSwapChain ();
	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();
	
	const auto  l_backBufferIndex = l_swapChain.FetchVALCurrentBackBufferIndex();
		  auto& l_backBufferList  = l_swapChain.GetMutableREFBackBufferList	  ();

	FWK_ASSERT_RETURN_IF_FAILED(l_backBufferList.empty(),										 "BackBufferListが空のため、BackBufferのPresent遷移に失敗しました。");
	FWK_ASSERT_RETURN_IF_FAILED(l_backBufferIndex >= static_cast<UINT>(l_backBufferList.size()), "BackBufferIndexが範囲外のため、BackBufferのPresent遷移に失敗しました。");

	auto& l_backBuffer = l_backBufferList[l_backBufferIndex];

	FWK_ASSERT_RETURN_IF_FAILED(l_backBuffer.m_rtvDescriptorIndex == Constant::k_invalidDescriptorIndex, "BackBufferのRTVDescriptorIndexが無効のため、BackBufferのPresent遷移に失敗しました。");

	// BackBufferをRENDERTARGET -> PRESENTへ遷移
	TransitionBackBufferResource(l_directCommandList, D3D12_RESOURCE_STATE_PRESENT, l_backBuffer);
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

void FWK::Graphics::RenderGraph::AddDrawRequestPerObject(const std::shared_ptr<DrawRequestPerObjectBase>& a_drawRequestPerObject)
{
	FWK_ASSERT_RETURN_IF_FAILED(!a_drawRequestPerObject, "DrawRequestPerObjectsが無効のため、DrawRequestPerObjectListへの登録に失敗しました。");

	const auto l_staticTypeID = a_drawRequestPerObject->GetREFRuntimeTypeINFO().k_staticTypeID;

	FWK_ASSERT_RETURN_IF_FAILED(m_drawRequestPerObjectMap.contains(l_staticTypeID), "同じ型のDrawRequestPerObjectを二重登録しようとしており、DrawRequestPerObjectMapへの登録に失敗しました。");

	m_drawRequestPerObjectList.emplace_back(a_drawRequestPerObject);
	m_drawRequestPerObjectMap.try_emplace  (l_staticTypeID, a_drawRequestPerObject);
}

void FWK::Graphics::RenderGraph::ClearBackBuffer(const ResourceContext& a_resourceContext, Renderer& a_renderer) const
{
		  auto& l_swapChain			= a_renderer.GetMutableREFSwapChain		   ();
	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList       ();
	const auto& l_rtvDescriptorPool = a_resourceContext.GetREFRTVDescriptorPool();
	const auto& l_renderArea		= a_renderer.GetREFRenderArea			   ();

	const auto  l_backBufferIndex = l_swapChain.FetchVALCurrentBackBufferIndex();
		  auto& l_backBufferList  = l_swapChain.GetMutableREFBackBufferList	  ();

	FWK_ASSERT_RETURN_IF_FAILED(l_backBufferList.empty(), "BackBufferListが空のため、BackBufferのClearに失敗しました。");
	FWK_ASSERT_RETURN_IF_FAILED(l_backBufferIndex >= static_cast<UINT>(l_backBufferList.size()), "BackBufferIndexが範囲外のため、BackBufferのClearに失敗しました。");

	auto& l_backBuffer = l_backBufferList[l_backBufferIndex];

	FWK_ASSERT_RETURN_IF_FAILED(l_backBuffer.m_rtvDescriptorIndex == Constant::k_invalidDescriptorIndex, "BackBufferのRTVDescriptorIndexが無効のため、BackBufferのClearに失敗しました。");

	// BackBufferをPRESENT -> RENDERTARGET
	TransitionBackBufferResource(l_directCommandList, D3D12_RESOURCE_STATE_RENDER_TARGET, l_backBuffer);

	// ビューポート、シザー矩形の設定
	l_directCommandList.SetupRenderArea(l_renderArea);

	// このバックバッファを描画先として設定する
	l_directCommandList.SetupRenderTarget(l_rtvDescriptorPool, l_backBuffer.m_rtvDescriptorIndex);

	// 描画先に設定したBackBufferを指定色でClearする。
	l_directCommandList.ClearRenderTarget(l_rtvDescriptorPool, l_backBuffer.m_rtvDescriptorIndex);
}

void FWK::Graphics::RenderGraph::TransitionPassResource(const RenderGraphPassBase& a_pass, Renderer& a_renderer) const
{
	for (const auto& l_resourceAccess : a_pass.GetREFResourceAccessList())
	{
		if (TransitionBackBufferResource(l_resourceAccess, a_renderer))			     { continue; }
		if (TransitionRenderTargetPassTextureResource(l_resourceAccess, a_renderer)) { continue; }

		FWK_ASSERT_RETURN("RenderGraphResourceAccessに対応するリソースが存在しないため、自動リソース遷移に失敗しました。");
	}
}
bool FWK::Graphics::RenderGraph::TransitionBackBufferResource(const Struct::RenderGraphResourceAccess& a_resourceAccess, Renderer& a_renderer) const
{
	if (a_resourceAccess.m_resourceType != Enum::RenderGraphResourceType::BackBuffer) { return false; }

		  auto& l_swapChain			= a_renderer.GetMutableREFSwapChain ();
	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();
	const auto  l_afterState        = ConvertVALD3D12ResourceState	    (a_resourceAccess.m_usage);

	const auto l_backBufferIndex = l_swapChain.FetchVALCurrentBackBufferIndex();
		  auto& l_backBufferList = l_swapChain.GetMutableREFBackBufferList   ();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_backBufferList.empty(),										   "BackBufferListが空のため、BackBufferの自動リソース遷移に失敗しました。",      true);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_backBufferIndex >= static_cast<UINT>(l_backBufferList.size()), "BackBufferIndexが範囲外のため、BackBufferの自動リソース遷移に失敗しました。", true);

	auto& l_backBuffer = l_backBufferList[l_backBufferIndex];

	TransitionBackBufferResource(l_directCommandList, l_afterState, l_backBuffer);

	return true;
}
void FWK::Graphics::RenderGraph::TransitionBackBufferResource(const DirectCommandList& a_directCommandList, const D3D12_RESOURCE_STATES a_afterState, Struct::BackBuffer& a_backBuffer) const
{
	FWK_ASSERT_RETURN_IF_FAILED(!a_backBuffer.m_backBufferResource, "バックバッファリソースが無効になっており、バックバッファリソースの状態遷移に失敗しました。");

	a_directCommandList.TransitionResourceBarrier(a_backBuffer.m_backBufferResource, a_backBuffer.m_currentResourceState, a_afterState);

	// 状態遷移後の状態を格納
	a_backBuffer.m_currentResourceState = a_afterState;
}
bool FWK::Graphics::RenderGraph::TransitionRenderTargetPassTextureResource(const Struct::RenderGraphResourceAccess& a_resourceAccess, Renderer& a_renderere) const
{
	const auto& l_currentFrameResource = a_renderere.GetREFCurrentFrameResource().lock();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_currentFrameResource, "現在のFrameResourceが無効のため、RenderTargetPassTextureの自動リソース遷移に失敗しました。", false);

	const auto& l_renderGraphFrameResource = l_currentFrameResource->GetREFRenderGraphFrameResource   ();
	const auto& l_renderTargetPassTexture  = l_renderGraphFrameResource.FindVALRenderTargetPassTexture(a_resourceAccess.m_resourceType).lock();

	if (!l_renderTargetPassTexture) { return false; }

	auto& l_renderTargetTexture = l_renderTargetPassTexture->GetMutableREFRenderTargetTexture();

	const auto& l_gpuResource = l_renderTargetTexture.GetREFGPUResource();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_gpuResource.m_resource, "RenderTargetPassTextureのGPUResourceが無効のため、自動リソース遷移に失敗しました。", false);

	const auto& l_directCommandList = a_renderere.GetREFDirectCommandList();

	const auto l_beforeState = l_renderTargetTexture.GetVALCurrentResourceState();
	const auto l_afterState  = ConvertVALD3D12ResourceState					  (a_resourceAccess.m_usage);

	// 既に必要な状態ならResourceBarrierは不要
	if (l_beforeState == l_afterState) { return true; }

	l_directCommandList.TransitionResourceBarrier(l_gpuResource.m_resource, l_beforeState, l_afterState);

	// RenderGraph側でRenderTargetTextureのCPU側Stateを更新する
	l_renderTargetTexture.SetCurrentResourceState(l_afterState);

	return true;
}

D3D12_RESOURCE_STATES FWK::Graphics::RenderGraph::ConvertVALD3D12ResourceState(const Enum::RenderGraphResourceUsage a_usage) const
{
	switch (a_usage)
	{
		case Enum::RenderGraphResourceUsage::RenderTarget:
		{
			return D3D12_RESOURCE_STATE_RENDER_TARGET;
		}
		break;

		case Enum::RenderGraphResourceUsage::PixelShaderResource:
		{
			return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		}
		break;

		case Enum::RenderGraphResourceUsage::Present:
		{
			return D3D12_RESOURCE_STATE_PRESENT;
		}
		break;

		default: 
		{
			FWK_ASSERT_RETURN_VALUE("未対応のRenderGraphResourceUsageが指定されたため、D3D12_RESOURCE_STATESへの変換に失敗しました。", D3D12_RESOURCE_STATE_COMMON);

		}
		break;
	}
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