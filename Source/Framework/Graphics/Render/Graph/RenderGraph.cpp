#include "RenderGraph.h"

void FWK::Graphics::RenderGraph::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}
void FWK::Graphics::RenderGraph::Compile()
{
	// Passの依存関係を解決し、実行順に並び替える
	m_passSorter.SortPassList(m_passList);
}

void FWK::Graphics::RenderGraph::BeginFrame(const ResourceContext& a_resourceContext, Renderer& a_renderer) 
{
	// 使用、ポインタがnullのパスの削除
	RemoveExpiredPassList();
	
	for (const auto& l_computeRequestPerObject : m_computeRequestPerObjectList)
	{
		FWK_ASSERT_RETURN_IF(!l_computeRequestPerObject, "ComputeRequestPerObjectが無効のため、BeginFrame処理に失敗しました。");

		l_computeRequestPerObject->BeginFrame();
	}

	for (const auto& l_drawRequestPerObject : m_drawRequestPerObjectList)
	{
		FWK_ASSERT_RETURN_IF              (!l_drawRequestPerObject, "DrawRequestPerObjectが無効のため、BeginFrame処理に失敗しました。");
		l_drawRequestPerObject->BeginFrame();
	}

	// バックバッファのリソース状態をRENDER_TARGETに遷移、バックバッファのクリア、RTVのセットを行う
	BeginBackBuffer(a_resourceContext, a_renderer);

	// 現在のフレームリソースが持つPassTextureをClearする
	m_resourceClearer.ClearCurrentFramePassTextureList(a_resourceContext, a_renderer);
}
void FWK::Graphics::RenderGraph::Execute(const ResourceContext& a_resourceContext, Renderer& a_renderer)
{
	const auto& l_cbvSRVUAVDescriptorPool = a_resourceContext.GetREFCBVSRVUAVDescriptorPool();
	const auto& l_directCommandList       = a_renderer.GetREFDirectCommandList	           ();
	const auto& l_computeCommandList      = a_renderer.GetREFComputeCommandList            ();

	// GraphicsShaderからBindlessResourceを参照するため、
	// Direct Command ListへShaderVisibleDescriptorHeapを設定する
	l_directCommandList.SetupDescriptorHeap(l_cbvSRVUAVDescriptorPool);

	// ComputeShaderも同じDescriptor Heap内の
	// SRV/UAVDescriptorIndexを参照するため、
	// ComputeCommandListへも設定する
	l_computeCommandList.SetupDescriptorHeap(l_cbvSRVUAVDescriptorPool);

	for (const auto& l_pass : m_passList)
	{
		if (!l_pass) { continue; }
		
		// Pass実行前に、ResourceAccessのbeforeUsageへ遷移する
		m_resourceTransitioner.TransitionPassResourceBefore(*l_pass, a_renderer);

		// PassのWriteResourceを見て、RenderGraph側でRTVを自動セットする
		m_resourceBinder.SetupPassRenderTarget(a_resourceContext, *l_pass, a_renderer);

		l_pass->Execute(a_resourceContext, a_renderer, *this);

		// Pass実行後に、ResourceAccessのafterUsageへ遷移する
		m_resourceTransitioner.TransitionPassResourceAfter(*l_pass, a_renderer);
	}
}
void FWK::Graphics::RenderGraph::EndFrame(Renderer& a_renderer) const
{
		  auto& l_swapChain			= a_renderer.GetMutableREFSwapChain ();
	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();
	
	const auto  l_backBufferIndex = l_swapChain.FetchVALCurrentBackBufferIndex();
		  auto& l_backBufferList  = l_swapChain.GetMutableREFBackBufferList	  ();

	FWK_ASSERT_RETURN_IF(l_backBufferList.empty(),										  "BackBufferListが空のため、BackBufferのPresent遷移に失敗しました。");
	FWK_ASSERT_RETURN_IF(l_backBufferIndex >= static_cast<UINT>(l_backBufferList.size()), "BackBufferIndexが範囲外のため、BackBufferのPresent遷移に失敗しました。");

	auto& l_backBuffer = l_backBufferList[l_backBufferIndex];

	FWK_ASSERT_RETURN_IF(l_backBuffer.m_rtvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "BackBufferのRTVDescriptorIndexが無効のため、BackBufferのPresent遷移に失敗しました。");

	// BackBufferをRENDERTARGET -> PRESENTへ遷移
	// ImGuiとの連携の関係上明示的にリソース遷移を行う
	m_resourceTransitioner.TransitionBackBufferResource(l_directCommandList, D3D12_RESOURCE_STATE_PRESENT, l_backBuffer);
}

nlohmann::json FWK::Graphics::RenderGraph::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

void FWK::Graphics::RenderGraph::AddPass(std::unique_ptr<RenderGraphPassBase>&& a_pass)
{
	FWK_ASSERT_RETURN_IF(!a_pass, "RenderGraphPassが無効のため、PassListへの登録処理に失敗しました。");

	m_passList.emplace_back(std::move(a_pass));
}

void FWK::Graphics::RenderGraph::AddDrawRequestPass(const std::shared_ptr<DrawRequestPassBase>& a_drawRequestPass)
{
	FWK_ASSERT_RETURN_IF(!a_drawRequestPass, "DrawRequestPassが無効のため、DrawRequestPassListへの登録に失敗しました。");

	const auto l_staticTypeID = a_drawRequestPass->GetREFRuntimeTypeINFO().k_staticTypeID;

	FWK_ASSERT_RETURN_IF(m_drawRequestPassMap.contains(l_staticTypeID), "同じ型のDrawRequestPassを二重登録しようとしており、DrawRequestPassMapへの登録に失敗しました。");

	m_drawRequestPassList.emplace_back(a_drawRequestPass);
	m_drawRequestPassMap.try_emplace  (l_staticTypeID, a_drawRequestPass);
}
void FWK::Graphics::RenderGraph::AddComputeRequestPerObject(const std::shared_ptr<ComputeRequestPerObjectBase>& a_computeRequestPerObject)
{
	FWK_ASSERT_RETURN_IF(!a_computeRequestPerObject, "ComputeRequestPerObjectが無効のため、ComputeRequestPerObjectListへの登録に失敗しました。");

	const auto l_staticTypeID = a_computeRequestPerObject->GetREFRuntimeTypeINFO().k_staticTypeID;

	FWK_ASSERT_RETURN_IF(m_computeRequestPerObjectMap.contains(l_staticTypeID), "同じ型のComputeRequestPerObjectを二重登録しようとしており、ComputeRequestPerObjectMapへの登録に失敗しました。");

	m_computeRequestPerObjectList.emplace_back(a_computeRequestPerObject);
	m_computeRequestPerObjectMap.try_emplace  (l_staticTypeID, a_computeRequestPerObject);	
}
void FWK::Graphics::RenderGraph::AddDrawRequestPerObject(const std::shared_ptr<DrawRequestPerObjectBase>& a_drawRequestPerObject)
{
	FWK_ASSERT_RETURN_IF(!a_drawRequestPerObject, "DrawRequestPerObjectsが無効のため、DrawRequestPerObjectListへの登録に失敗しました。");

	const auto l_staticTypeID = a_drawRequestPerObject->GetREFRuntimeTypeINFO().k_staticTypeID;

	FWK_ASSERT_RETURN_IF(m_drawRequestPerObjectMap.contains(l_staticTypeID), "同じ型のDrawRequestPerObjectを二重登録しようとしており、DrawRequestPerObjectMapへの登録に失敗しました。");

	m_drawRequestPerObjectList.emplace_back(a_drawRequestPerObject);
	m_drawRequestPerObjectMap.try_emplace  (l_staticTypeID, a_drawRequestPerObject);
}

void FWK::Graphics::RenderGraph::BeginBackBuffer(const ResourceContext& a_resourceContext, Renderer& a_renderer) const
{
	      auto& l_swapChain			= a_renderer.GetMutableREFSwapChain ();
	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();
	const auto& l_rtvDescriptorPool = a_resourceContext.GetREFRTVDescriptorPool();

	const auto  l_backBufferIndex = l_swapChain.FetchVALCurrentBackBufferIndex();
		  auto& l_backBufferList  = l_swapChain.GetMutableREFBackBufferList	  ();

	FWK_ASSERT_RETURN_IF(l_backBufferList.empty(), "BackBufferListが空のため、BackBufferのClearに失敗しました。");
	FWK_ASSERT_RETURN_IF(l_backBufferIndex >= static_cast<UINT>(l_backBufferList.size()), "BackBufferIndexが範囲外のため、BackBufferのClearに失敗しました。");

	auto& l_backBuffer = l_backBufferList[l_backBufferIndex];

	FWK_ASSERT_RETURN_IF(l_backBuffer.m_rtvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "BackBufferのRTVDescriptorIndexが無効のため、BackBufferのClearに失敗しました。");

	// BackBufferをPRESENT -> RENDERTARGETに明示的に遷移
	m_resourceTransitioner.TransitionBackBufferResource(l_directCommandList, D3D12_RESOURCE_STATE_RENDER_TARGET, l_backBuffer);

	// このバックバッファを描画先として設定する
	l_directCommandList.SetupRenderTarget(l_rtvDescriptorPool, l_backBuffer.m_rtvDescriptorIndex);

	// 描画先に設定したBackBufferを指定色でClearする。
	l_directCommandList.ClearRenderTarget(l_rtvDescriptorPool, l_backBuffer.m_rtvDescriptorIndex);
}

void FWK::Graphics::RenderGraph::RemoveExpiredPassList()
{
	std::size_t l_index = 0ULL;

	while (l_index < m_passList.size())
	{
		if (const auto& l_pass = m_passList[l_index];
			l_pass)
		{
			++l_index;
			continue;
		}

		std::swap          (m_passList[l_index], m_passList.back());
		m_passList.pop_back();
	}
}