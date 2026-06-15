#include "RenderGraph.h"

void FWK::Graphics::RenderGraph::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}
void FWK::Graphics::RenderGraph::Compile()
{
	// 現在のパス総数を取得
	const auto l_passCount = m_passList.size();

	// パス数が2未満ならパスが一個か0個なのでトポロジカルソートをする必要がないのでreturn
	if (l_passCount < k_minPassCountToResolveExecutionOrder) { return; }

	// 各Passが「どのPassの前に実行されるべきか」を持つリスト
	std::vector<std::vector<std::size_t>> l_passDependencyList = {};

	// 各Passに対して「自分より前に実行されなければならないPassの数」を持つリスト
	std::vector<std::size_t> l_passInDegreeList = {};

	// パス数分依存関係リストを作成する
	l_passDependencyList.resize(l_passCount);

	// パス数分入次数リストを作成する(初期値は0)
	l_passInDegreeList.resize(l_passCount, k_emptyPassInDegree);

	// Pass同士を総当たりで比較してResourceAccessから依存関係を作る
	// l_beforePassIndexより後ろのPassだけを見ることで同じ組み合わせを二回比較しないようにしている
	for (std::size_t l_beforePassIndex = 0ULL; l_beforePassIndex < l_passCount; ++l_beforePassIndex)
	{
		if (!m_passList[l_beforePassIndex]) { continue; }

		for (std::size_t l_afterPassIndex = l_beforePassIndex + k_nextPassIndexOffset; l_afterPassIndex < l_passCount; ++l_afterPassIndex)
		{
			if (!m_passList[l_afterPassIndex]) { continue; }

			// 2つのPassが持つResourceAccessを比較し、必要であれば依存辺を追加する
			// 例 : ScreenSpritePass : Write SceneColor
			//      FinalPresentPass : Read  SceneColor
			// この場合、ScreenSpritePassの結果をFinalPresentPassが読むので、ScreenSpritePass -> FinalPresentPassという依存関係を作る
			AddPassResourceDependencyEdge(l_beforePassIndex, 
										  l_afterPassIndex,
										  l_passDependencyList,
										  l_passInDegreeList);
		}
	}

	// 入次数が0のPassを処理するためのキュー
	// 入次数0とは、「自分より前に実行すべきPassが無い」という意味。
	std::queue<std::size_t> l_visitQueue = {};

	for (std::size_t l_passIndex = 0ULL; l_passIndex < l_passCount; ++l_passIndex)
	{
		// 依存元が残っているPassは、まだ実行順に追加できない。
		if (l_passInDegreeList[l_passIndex] != k_emptyPassInDegree) { continue; }

		// 依存元がないPassは、先に実行できる候補としてキューへ追加する
		l_visitQueue.push(l_passIndex);
	}

	// 実際のPassではなく、元のm_passList上のIndexを格納するリスト
	std::vector<std::size_t> l_sortedPassIndexList = {};

	// 最大でパス数分のIndexが入るため、先に容量を確保する。
	l_sortedPassIndexList.reserve(l_passCount);

	// キューが空になるまで実行
	while (!l_visitQueue.empty())
	{
		// 現在実行順序に追加できるPassIndexを取り出す
		const auto l_currentPassIndex = l_visitQueue.front();
		
		// 取得したPassIndexはこれから処理するため、Queueから取り除く
		l_visitQueue.pop();

		// 現在のPassを、解決済みの実行順リストへ追加する。
		l_sortedPassIndexList.emplace_back(l_currentPassIndex);

		// 現在のPassの後に実行されるべきPassを確認する。
		for (const auto l_nextPassIndex : l_passDependencyList[l_currentPassIndex])
		{
			// 入次数が既に0なら、これ以上減らすとstd::size_tがアンダーフローする。
			// そのため、ここで不正な依存関係として検出する
			FWK_ASSERT_RETURN_IF_FAILED(l_passInDegreeList[l_nextPassIndex] == k_emptyPassInDegree, "RenderGraphPassの入次数が不正なため、Pass実行順の解決に失敗しました。");

			// 現在のPassを実行順に追加したので
			// そのPassに依存していた次のPassの入次数を一つ減らす
			--l_passInDegreeList[l_nextPassIndex];

			// まだ依存元が残っているなら、次のPassはまだ実行できない。
			if (l_passInDegreeList[l_nextPassIndex] != k_emptyPassInDegree) { continue; }

			// 入次数が0になったので次に実行できる候補としてキューへ追加する。
			l_visitQueue.push(l_nextPassIndex);
		}
	}

	// 全パスを実行順リストへ追加できなかった場合、循環依存がある。
	// 例 PassA -> PassB
	//	  PassB -> PassA
	// この場合、どちらを先に実行するべきか決めれない
	FWK_ASSERT_RETURN_IF_FAILED(l_sortedPassIndexList.size() != l_passCount, "RenderGraphの依存順が循環しているため、Pass実行順の解決に失敗しました。");

	// 解決した順番に従って、新しいPassListを作る
	std::vector<std::unique_ptr<RenderGraphPassBase>> l_sortedPassList = {};

	l_sortedPassList.reserve(l_passCount);

	for (const auto l_sortedPassIndex : l_sortedPassIndexList)
	{
		l_sortedPassList.emplace_back(std::move(m_passList[l_sortedPassIndex]));
	}

	// 並び替え済みのPassListに差し替える
	m_passList = std::move(l_sortedPassList);
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

	// バックバッファのリソース状態をRENDER_TARGETに遷移、バックバッファのクリア、RTVのセットを行う
	BeginBackBuffer(a_resourceContext, a_renderer);

	// 現在のFrameResourceが持つ中間RenderTargetをクリアする
	ClearCurrentFrameRenderTargetPassTextureList(a_resourceContext, a_renderer);
}
void FWK::Graphics::RenderGraph::Execute(const ResourceContext& a_resourceContext, Renderer& a_renderer)
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

		// PassのWriteResourceを見て、RenderGraph側でRTVを自動セットする
		SetupPassRenderTarget(a_resourceContext, *l_pass, a_renderer);

		l_pass->Execute(a_resourceContext, a_renderer, *this);
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

void FWK::Graphics::RenderGraph::BeginBackBuffer(const ResourceContext& a_resourceContext, Renderer& a_renderer) const
{
	      auto& l_swapChain			= a_renderer.GetMutableREFSwapChain ();
	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();
	const auto& l_rtvDescriptorPool = a_resourceContext.GetREFRTVDescriptorPool();

	const auto  l_backBufferIndex = l_swapChain.FetchVALCurrentBackBufferIndex();
		  auto& l_backBufferList  = l_swapChain.GetMutableREFBackBufferList	  ();

	FWK_ASSERT_RETURN_IF_FAILED(l_backBufferList.empty(), "BackBufferListが空のため、BackBufferのClearに失敗しました。");
	FWK_ASSERT_RETURN_IF_FAILED(l_backBufferIndex >= static_cast<UINT>(l_backBufferList.size()), "BackBufferIndexが範囲外のため、BackBufferのClearに失敗しました。");

	auto& l_backBuffer = l_backBufferList[l_backBufferIndex];

	FWK_ASSERT_RETURN_IF_FAILED(l_backBuffer.m_rtvDescriptorIndex == Constant::k_invalidDescriptorIndex, "BackBufferのRTVDescriptorIndexが無効のため、BackBufferのClearに失敗しました。");

	// BackBufferをPRESENT -> RENDERTARGET
	TransitionBackBufferResource(l_directCommandList, D3D12_RESOURCE_STATE_RENDER_TARGET, l_backBuffer);

	// このバックバッファを描画先として設定する
	l_directCommandList.SetupRenderTarget(l_rtvDescriptorPool, l_backBuffer.m_rtvDescriptorIndex);

	// 描画先に設定したBackBufferを指定色でClearする。
	l_directCommandList.ClearRenderTarget(l_rtvDescriptorPool, l_backBuffer.m_rtvDescriptorIndex);
}

void FWK::Graphics::RenderGraph::ClearCurrentFrameRenderTargetPassTextureList(const ResourceContext& a_resourceContext, const Renderer& a_renderer) const
{
	const auto& l_currentFrameResource = a_renderer.GetREFCurrentFrameResource().lock();

	FWK_ASSERT_RETURN_IF_FAILED(!l_currentFrameResource, "現在のFrameResourceが無効のため、RenderTargetPassTextureListのClearに失敗しました。");

	const auto& l_renderGraphFrameResource = l_currentFrameResource->GetREFRenderGraphFrameResource();

	for (const auto& l_renderTargetPassTexture : l_renderGraphFrameResource.GetREFRenderTargetPassTextureList())
	{
		FWK_ASSERT_RETURN_IF_FAILED(!l_renderTargetPassTexture, "RenderTargetPassTextureが無効のため、RenderTargetPassTextureのClearに失敗しました。");

		// クリアを毎フレームすべきでないレンダーターゲットテクスチャはクリアしない
		if (l_renderTargetPassTexture->GetVALIsSkipClearOnBeginFrame()) { continue; }

		FWK_ASSERT_RETURN_IF_FAILED(!ClearRenderTargetPassTexture(a_resourceContext, a_renderer, *l_renderTargetPassTexture), "RenderTargetPassTextureのClearに失敗しました。");
	}
}
bool FWK::Graphics::RenderGraph::ClearRenderTargetPassTexture(const ResourceContext & a_resourceContext, const Renderer & a_renderer, RenderTargetPassTexture & a_renderTargetPassTexture) const
{
	auto& l_renderTargetTexture = a_renderTargetPassTexture.GetMutableREFRenderTargetTexture();

	const auto& l_gpuResource = l_renderTargetTexture.GetREFGPUResource();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_gpuResource.m_resource,																  "RenderTargetPassTextureのGPUResourceが無効のため、Clearに失敗しました。",        false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_renderTargetTexture.GetVALRTVDescriptorIndex() == Constant::k_invalidDescriptorIndex, "RenderTargetPassTextureのRTVDescriptorIndexが無効のため、Clearに失敗しました。", false);

	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList       ();
	const auto& l_rtvDescriptorPool = a_resourceContext.GetREFRTVDescriptorPool();

	// リソースをレンダーターゲットに状態遷移して現在のステートとして格納
	if (const auto l_beforeState = l_renderTargetTexture.GetVALCurrentResourceState();
		l_beforeState != D3D12_RESOURCE_STATE_RENDER_TARGET)
	{
		l_directCommandList.TransitionResourceBarrier(l_gpuResource.m_resource, l_beforeState, D3D12_RESOURCE_STATE_RENDER_TARGET);
		l_renderTargetTexture.SetCurrentResourceState(D3D12_RESOURCE_STATE_RENDER_TARGET);
	}

	// RenderTargetPassTexture自身が持つClearColorでClearする。
	// SceneColorならGraphicsCONFIG.jsonのSceneColor.ClearColorが反映される。
	l_directCommandList.ClearRenderTarget(l_rtvDescriptorPool, l_renderTargetTexture.GetVALRTVDescriptorIndex(), a_renderTargetPassTexture.GetREFClearColor());

	return true;
}

bool FWK::Graphics::RenderGraph::SetupBackBufferRenderTarget(const ResourceContext& a_resourceContext, const Renderer& a_renderer, const Struct::RenderGraphResourceAccess& a_resourceAccess) const
{
	if (a_resourceAccess.m_resourceType != Enum::RenderGraphResourceType::BackBuffer) { return false; }

	const auto& l_swapChain         = a_renderer.GetREFSwapChain        ();
	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();

	const auto  l_backBufferIndex = l_swapChain.FetchVALCurrentBackBufferIndex();
	const auto& l_backBufferList  = l_swapChain.GetREFBackBufferList		  ();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_backBufferList.empty(),										   "BackBufferListが空のため、BackBufferの描画先設定に失敗しました。",      false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_backBufferIndex >= static_cast<UINT>(l_backBufferList.size()), "BackBufferIndexが範囲外のため、BackBufferの描画先設定に失敗しました。", false);

	const auto& l_backBuffer = l_backBufferList[l_backBufferIndex];

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_backBuffer.m_rtvDescriptorIndex == Constant::k_invalidDescriptorIndex, "BackBufferのRTVDescriptorIndexが無効のため、BackBufferの描画先設定に失敗しました。", false);

	const auto& l_rtvDescriptorPool = a_resourceContext.GetREFRTVDescriptorPool();
	const auto& l_renderArea        = a_renderer.GetREFRenderArea			   ();

	// ビューポート、シザー矩形のセット、バックバッファをOMにセット
	l_directCommandList.SetupRenderArea  (l_renderArea);
	l_directCommandList.SetupRenderTarget(l_rtvDescriptorPool, l_backBuffer.m_rtvDescriptorIndex);

	return true;
}
bool FWK::Graphics::RenderGraph::SetupRenderTargetPassTextureRenderTarget(const ResourceContext& a_resourceContext, const Renderer& a_renderer, const Struct::RenderGraphResourceAccess& a_resourceAccess) const
{
	const auto& l_currentFrameResource = a_renderer.GetREFCurrentFrameResource().lock();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_currentFrameResource, "現在のFrameResourceが無効のため、RenderTargetPassTextureの描画先設定に失敗しました。", false);

	const auto& l_renderGraphFrameResource = l_currentFrameResource->GetREFRenderGraphFrameResource   ();
	const auto& l_renderTargetPassTexture  = l_renderGraphFrameResource.FindVALRenderTargetPassTexture(a_resourceAccess.m_resourceType).lock();

	if (!l_renderTargetPassTexture) { return false; }

	const auto& l_renderTargetTexture = l_renderTargetPassTexture->GetREFRenderTargetTexture();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_renderTargetTexture.GetVALRTVDescriptorIndex() == Constant::k_invalidDescriptorIndex, "RenderTargetPassTextureのRTVDescriptorIndexが無効のため、描画先設定に失敗しました。", false);

	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList       ();
	const auto& l_rtvDescriptorPool = a_resourceContext.GetREFRTVDescriptorPool();
	const auto& l_renderArea        = a_renderer.GetREFRenderArea			   ();

	l_directCommandList.SetupRenderArea  (l_renderArea);
	l_directCommandList.SetupRenderTarget(l_rtvDescriptorPool,  l_renderTargetTexture.GetVALRTVDescriptorIndex());
	
	return true;
}
void FWK::Graphics::RenderGraph::SetupPassRenderTarget(const ResourceContext & a_resourceContext, const RenderGraphPassBase & a_pass, const Renderer & a_renderer) const
{
	for (const auto& l_resourceAccess : a_pass.GetREFResourceAccessList())
	{
		// レンダーターゲットに書き込まなければcontinue
		if (!IsWriteRenderTargetAccess(l_resourceAccess)) { continue; }

		if (SetupBackBufferRenderTarget(a_resourceContext, a_renderer, l_resourceAccess))			   { continue; }
		if (SetupRenderTargetPassTextureRenderTarget(a_resourceContext, a_renderer, l_resourceAccess)) { continue; }

		FWK_ASSERT_RETURN("ResourceAccessに対応するRenderTargetが存在しないため、Passの描画先設定に失敗しました。");
	}
}

bool FWK::Graphics::RenderGraph::IsWriteRenderTargetAccess(const Struct::RenderGraphResourceAccess& a_resourceAccess) const
{
	if (a_resourceAccess.m_accessType != Enum::RenderGraphAccessType::Write ||
		a_resourceAccess.m_usage      != Enum::RenderGraphResourceUsage::RenderTarget)	
	{
		return false; 
	}
	
	return true;
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

	const auto  l_backBufferIndex = l_swapChain.FetchVALCurrentBackBufferIndex();
		  auto& l_backBufferList  = l_swapChain.GetMutableREFBackBufferList   ();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_backBufferList.empty(),										   "BackBufferListが空のため、BackBufferの自動リソース遷移に失敗しました。",      true);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_backBufferIndex >= static_cast<UINT>(l_backBufferList.size()), "BackBufferIndexが範囲外のため、BackBufferの自動リソース遷移に失敗しました。", true);

	auto& l_backBuffer = l_backBufferList[l_backBufferIndex];

	// バックバッファのリソース状態の遷移
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
bool FWK::Graphics::RenderGraph::TransitionRenderTargetPassTextureResource(const Struct::RenderGraphResourceAccess& a_resourceAccess, const Renderer& a_renderer) const
{
	const auto& l_currentFrameResource = a_renderer.GetREFCurrentFrameResource().lock();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_currentFrameResource, "現在のFrameResourceが無効のため、RenderTargetPassTextureの自動リソース遷移に失敗しました。", false);

	// 現在のフレームリソースからレンダーターゲットパステクスチャを取得
	const auto& l_renderGraphFrameResource = l_currentFrameResource->GetREFRenderGraphFrameResource   ();
	const auto& l_renderTargetPassTexture  = l_renderGraphFrameResource.FindVALRenderTargetPassTexture(a_resourceAccess.m_resourceType).lock();

	if (!l_renderTargetPassTexture) { return false; }

	auto& l_renderTargetTexture = l_renderTargetPassTexture->GetMutableREFRenderTargetTexture();

	// リソース遷移の対象になるGPUResourceを取得
	const auto& l_gpuResource = l_renderTargetTexture.GetREFGPUResource();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_gpuResource.m_resource, "RenderTargetPassTextureのGPUResourceが無効のため、自動リソース遷移に失敗しました。", false);

	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();

	const auto l_beforeState = l_renderTargetTexture.GetVALCurrentResourceState();
	const auto l_afterState  = ConvertVALD3D12ResourceState					  (a_resourceAccess.m_usage);

	// 既に必要な状態ならResourceBarrierは不要
	if (l_beforeState == l_afterState) { return true; }

	// リソース状態の遷移
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

void FWK::Graphics::RenderGraph::AddPassResourceDependencyEdge(const std::size_t&							a_beforePassIndex, 
															   const std::size_t&							a_afterPassIndex, 
																	 std::vector<std::vector<std::size_t>>& a_passDependencyList, 
																     std::vector<std::size_t>&				a_passInDegreeList)
{
	FWK_ASSERT_RETURN_IF_FAILED(a_beforePassIndex >= m_passList.size() ||
								a_afterPassIndex  >= m_passList.size(),
								"Before,Afterのパスインデックスが容量を超えています、RenderGraphPassの依存関係作成に失敗しました。");

	const auto& l_beforePass = m_passList[a_beforePassIndex];
	const auto& l_afterPass  = m_passList[a_afterPassIndex];

	FWK_ASSERT_RETURN_IF_FAILED(!l_beforePass, "BeforePassが無効のため、RenderGraphPassの依存関係作成に失敗しました。");
	FWK_ASSERT_RETURN_IF_FAILED(!l_afterPass,  "AfterPassが無効のため、RenderGraphPassの依存関係作成に失敗しました。");

	const auto& l_beforeResourceAccessList = l_beforePass->GetREFResourceAccessList();
	const auto& l_afterResourceAccessList  = l_afterPass->GetREFResourceAccessList ();

	for (const auto& l_beforeResourceAccess : l_beforeResourceAccessList)
	{
		for (const auto& l_afterRenderAccess : l_afterResourceAccessList)
		{
			// 違うRenderGraphResourceType同士なら依存関係は作らない。
			// SceneColorとBackBufferは別リソースなので、
			// それだけで前後関係を決めない
			if (!IsSameRenderGraphResource(l_beforeResourceAccess, l_afterRenderAccess)) { continue; }

			const auto l_isBeforeRead  = IsReadResourceAccess (l_beforeResourceAccess);
			const auto l_isBeforeWrite = IsWriteResourceAccess(l_beforeResourceAccess);

			const auto l_isAfterRead  = IsReadResourceAccess (l_afterRenderAccess);
			const auto l_isAfterWrite = IsWriteResourceAccess(l_afterRenderAccess);

			// beforeが書いてafterが読む場合
			// 例 : 
			// before = ScreenSpritePass : Write SceneColor
			// after  = FinalPresentPass : Read SceneColor
			// afterはbeforeの書き込み結果を読む必要があるため、
			// before -> afterの依存関係を作る
			if (l_isBeforeWrite && l_isAfterRead)
			{
				AddPassDependencyEdge(a_beforePassIndex,
									  a_afterPassIndex,
									  a_passDependencyList,
									  a_passInDegreeList);

				continue;
			}

			// beforeが呼んだafterが書く場合。
			// 例 : 
			// before = FinalPresentPass : Read  SceneColor
			// after  = ScreenSpritePass : Write SceneColor
			// json上ではFinalPresentPassが先に書かれていても、
			// 実際にはScreenSpritePassがSceneColorを書いた後でないと読めない。
			// そのため after->beforeの依存関係を作る
			if (l_isBeforeRead && l_isAfterWrite)
			{
				AddPassDependencyEdge(a_afterPassIndex,
									  a_beforePassIndex,
									  a_passDependencyList,
									  a_passInDegreeList);

				continue;
			}

			// 両方が同じリソースを書く場合
			// 例 : 
			// ModelPass  : Write SceneColor 
			// SpritePass : Write SceneColor
			// ResourceAccessだけでは、どちらを先に書くべきか判断できない。
			// jsonに書かれた元の順番を維持する
			if (l_isBeforeWrite && l_isAfterWrite)
			{
				AddPassDependencyEdge(a_beforePassIndex, 
									  a_afterPassIndex,
									  a_passDependencyList,
									  a_passInDegreeList);

				continue;
			}
 		}
	}
}
void FWK::Graphics::RenderGraph::AddPassDependencyEdge(const std::size_t&							a_beforePassIndex, 
													   const std::size_t&							a_afterPassIndex, 
															 std::vector<std::vector<std::size_t>>& a_passDependencyList, 
															 std::vector<std::size_t>&			    a_passInDegreeList) const
{
	// 自分自身への依存は意味がないため追加しない
	if (a_beforePassIndex == a_afterPassIndex) { return; }

	// 同じ依存辺を二重に追加しない。
	// ResourceAccessの組み合わせによっては、同じPass間の依存が複数回見つかる可能性がある。
	for (const auto l_nextPassIndex : a_passDependencyList[a_beforePassIndex])
	{
		if (l_nextPassIndex != a_afterPassIndex) { continue; }

		return;
	}

	// before -> afterの依存辺を追加する
	// これは「beforeの後にafterを実行する必要がある」という意味。
	a_passDependencyList[a_beforePassIndex].emplace_back(a_afterPassIndex);

	// afterはbeforeに依存するため、afterの入次数を増やす
	++a_passInDegreeList[a_afterPassIndex];
}

bool FWK::Graphics::RenderGraph::IsSameRenderGraphResource(const Struct::RenderGraphResourceAccess& a_lhs, const Struct::RenderGraphResourceAccess& a_rhs) const
{
	// InvalidやCountは実在するRenderGraphResourceではないため、依存判定の対象外にする
	if (a_lhs.m_resourceType == Enum::RenderGraphResourceType::Invalid) { return false; }
	if (a_lhs.m_resourceType == Enum::RenderGraphResourceType::Count)   { return false; }

	if (a_rhs.m_resourceType == Enum::RenderGraphResourceType::Invalid) { return false; }
	if (a_rhs.m_resourceType == Enum::RenderGraphResourceType::Count)   { return false; }

	// 同じRenderGraphResourceTypeを参照している場合だけ、依存関係の候補にする
	return a_lhs.m_resourceType == a_rhs.m_resourceType;
}

bool FWK::Graphics::RenderGraph::IsReadResourceAccess(const Struct::RenderGraphResourceAccess& a_resourceAccess) const
{
	return a_resourceAccess.m_accessType == Enum::RenderGraphAccessType::Read;
}
bool FWK::Graphics::RenderGraph::IsWriteResourceAccess(const Struct::RenderGraphResourceAccess& a_resourceAccess) const
{
	return a_resourceAccess.m_accessType == Enum::RenderGraphAccessType::Write;
}