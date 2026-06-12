#include "Renderer.h"

void FWK::Graphics::Renderer::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}
bool FWK::Graphics::Renderer::PostDeserialize(const Device&						  a_device, 
											  const Window&						  a_window, 
											  const Factory&					  a_factory,
											  const ShaderCompiler&				  a_shaderCompiler,
													TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool)
{
	// フレームリソースがないとコマンドアロケーターを使えないため"return"
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(m_frameResourceList.empty(), "フレームリソースリストが空になっており、フレームリソース作成処理に失敗しました。", false);

	for (const auto& l_frameResource : m_frameResourceList)
	{
		if (!l_frameResource) { continue; }

		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_frameResource->Create(a_device), "フレームリソースの作成処理に失敗しました。", false);
	}

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_directCommandQueue.Create(a_device), "ダイレクトコマンドキューの作成処理に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_directCommandList.Create(a_device),  "ダイレクトコマンドリストの作成処理に失敗しました。", false);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_swapChain.Create(a_window,
														  a_device,
														  a_factory, m_directCommandQueue, 
														  a_rtvDescriptorPool), 
														  "ダイレクトコマンドリストの作成処理に失敗しました。",
														  false);

	// ルートシグネチャの作成処理
	for (const auto& [l_type, l_rootSignature] : m_rootSignatureMap)
	{
		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_rootSignature,					  "RootSignatureが無効のため、RootSignatureの作成に失敗しました。", false);
		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_rootSignature->Create(a_device), "RootSignatureの作成処理に失敗しました。",						    false);
	}

	// パイプラインステートの作成処理
	for (const auto& [l_type, l_pipelineState] : m_pipelineStateMap)
	{
		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_pipelineState,											   "PipelineStateが無効のため、PipelineStateの作成に失敗しました。", false);
		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_pipelineState->Create(a_device, a_shaderCompiler, *this), "PipelineStateの作成処理に失敗しました。",						 false);
	}

	// 画面解像度に合ったビューポート、シザー矩形を作成する
	m_renderArea.SetupRenderArea(m_swapChain);

	// ALT + ENTERキーで排他フルスクリーン設定が反映されないようにする
	m_swapChain.PostCreateSetup(a_window.GetREFHWND(), a_factory);

	// 定数バッファを書くパスに送信
	m_renderArea.SyncSpritePassDrawRequest(m_renderGraph);

	// 最初に使うフレームリソースをキャッシュ
	m_currentFrameResource = m_frameResourceList[k_initialFrameResourceIndex];

	return true;
}

void FWK::Graphics::Renderer::BeginFrame(const ResourceContext& a_resourceContext)
{
	// 現在のフレームリソースの定数バッファのインデックスの初期化
	const auto& l_currentFrameResource = m_currentFrameResource.lock();

	FWK_ASSERT_RETURN_IF_FAILED(!l_currentFrameResource, "フレームリソースの取得に失敗しており、描画開始処理に失敗しました。");

	ResetCommandObjects(*l_currentFrameResource);

	// GPUがこのフレームリソースを使い終わってから、
	// ConstantBufferUploaderの書き込みインデックスを先頭に戻す
	l_currentFrameResource->BeginFrame();

	// リソース遷移の実行
	m_renderGraph.BeginFrame(a_resourceContext, *l_currentFrameResource, *this);
}
void FWK::Graphics::Renderer::Execute(const ResourceContext& a_resourceContext)
{
	const auto& l_currentFrameResource = m_currentFrameResource.lock();

	FWK_ASSERT_RETURN_IF_FAILED(!l_currentFrameResource, "フレームリソースの取得に失敗しており、描画開始処理に失敗しました。");

	m_renderGraph.Execute(a_resourceContext, *l_currentFrameResource, *this);
}
void FWK::Graphics::Renderer::EndFrame()
{
	const auto& l_currentFrameResource = m_currentFrameResource.lock();

	FWK_ASSERT_RETURN_IF_FAILED(!l_currentFrameResource, "フレームリソースの取得に失敗しており、描画終了処理に失敗しました。");

	const auto& l_commandAllocator = l_currentFrameResource->GetREFDirectCommandAllocator();

	FWK_ASSERT_RETURN_IF_FAILED(!l_commandAllocator, "ダイレクトコマンドアロケータが無効になっており、描画終了処理に失敗しました。");

	// スワップチェインのリソース状態遷移(RENDER_TARGET -> PRESENT)
	m_renderGraph.EndFrame(*this);

	// コマンドリストへの命令記録を終了
	m_directCommandList.Close();

	// 描画命令を実行
	m_directCommandQueue.ExecuteCommandLists(m_directCommandList);

	// フェンス値を更新
	m_directCommandQueue.SignalAndTrackAllocator(*l_commandAllocator);

	// 画面描画
	m_swapChain.Present();

	// 次のフレームで使用するフレームリソースを決める
	DecideNextFrameUseFrameResource();
}

nlohmann::json FWK::Graphics::Renderer::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

void FWK::Graphics::Renderer::Resize(const Device&			             a_device,
									 const ResourceReleaseContext&       a_resourceReleaseContext, 
									 const Struct::ClientSize&	         a_clientSize, 
										   TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool)
{
	// スワップチェインのリサイズ前にGPUとの同期をとるなど必要な処理を行う
	PrepareForSwapChainResize();

	// バックバッファのリサイズを行う
	FWK_ASSERT_RETURN_IF_FAILED(!m_swapChain.Resize(a_device,
													a_resourceReleaseContext, 
													a_clientSize, 
													a_rtvDescriptorPool), 
													"リサイズ処理に失敗しました。");

	// スワップチェインリサイズ後にレンダーエリアを作成
	m_renderArea.SetupRenderArea(m_swapChain);
}

void FWK::Graphics::Renderer::AddFrameResource(const std::shared_ptr<FrameResource>& a_frameResource)
{
	FWK_ASSERT_RETURN_IF_FAILED(!a_frameResource, "FrameResourceが無効のため、FrameResourceListへの登録に失敗しました。");

	m_frameResourceList.emplace_back(a_frameResource);
}
void FWK::Graphics::Renderer::AddRootSignature(const std::shared_ptr<RootSignature>& a_rootSignature, const Enum::RootSignatureType a_rootSignatureType)
{
	FWK_ASSERT_RETURN_IF_FAILED(!a_rootSignature,										 "RootSignatureが無効のため、RootSignatureMapへの登録に失敗しました。");
	FWK_ASSERT_RETURN_IF_FAILED(a_rootSignatureType == Enum::RootSignatureType::Invalid, "RootSignatureが無効な種類のため、RootSignatureMapへの登録に失敗しました。");

	m_rootSignatureMap.try_emplace(a_rootSignatureType, a_rootSignature);	
}
void FWK::Graphics::Renderer::AddPipelineState(const std::shared_ptr<PipelineState>& a_pipelineState, const Enum::PipelineStateType a_pipelineStateType)
{
	FWK_ASSERT_RETURN_IF_FAILED(!a_pipelineState,										 "PipelineStateが無効のため、PipelineStateMapへの登録に失敗しました。");
	FWK_ASSERT_RETURN_IF_FAILED(a_pipelineStateType == Enum::PipelineStateType::Invalid, "PipelineStateが無効な種類のため、PipelineStateMapへの登録に失敗しました。");

	m_pipelineStateMap.try_emplace(a_pipelineStateType, a_pipelineState);
}

std::weak_ptr<FWK::Graphics::RootSignature> FWK::Graphics::Renderer::FindVALRootSignature(const Enum::RootSignatureType a_rootSignatureType) const
{
	const auto& l_itr = m_rootSignatureMap.find(a_rootSignatureType);

	if (l_itr == m_rootSignatureMap.end()) { return {}; }

	return l_itr->second;
}
std::weak_ptr<FWK::Graphics::PipelineState> FWK::Graphics::Renderer::FindVALPipelineState(const Enum::PipelineStateType a_pipelineStateType) const
{
	const auto& l_itr = m_pipelineStateMap.find(a_pipelineStateType);

	if (l_itr == m_pipelineStateMap.end()) { return {}; }

	return l_itr->second;
}

void FWK::Graphics::Renderer::ResetCommandObjects(const FrameResource& a_frameResource)
{
	const auto& l_commandAllocator = a_frameResource.GetREFDirectCommandAllocator();

	FWK_ASSERT_RETURN_IF_FAILED(!l_commandAllocator, "ダイレクトコマンドアロケータが無効になっており、描画開始処理に失敗しました。");

	// コマンドアロケータからGPU処理が終わっているかどうかを確かめGPUの処理が終わっていればWait
	m_directCommandQueue.EnsureAllocatorAvailable(*l_commandAllocator);

	// GPU同期処理が終わってからコマンドリスト、アロケータをリセット
	l_commandAllocator->Reset();
	m_directCommandList.Reset(*l_commandAllocator);
}

void FWK::Graphics::Renderer::DecideNextFrameUseFrameResource()
{
	// 容量を超えないように次のフレームで使用するインデックスを計算
	// 現在のインデックス / フレームリソースの総数の余りを算出しているので
	// 絶対にインデックスのオーバーフローが起きないことは保証されている
	m_currentFrameResourceIndex = (m_currentFrameResourceIndex + k_frameResourceIndexIncrement) % m_frameResourceList.size();

	// 次に使用するフレームリソースをキャッシュしておく
	m_currentFrameResource = m_frameResourceList[m_currentFrameResourceIndex];
}

bool FWK::Graphics::Renderer::PrepareForSwapChainResize()
{
	// ResizeBuffers()の前に、GPUが直前までの描画命令を使い終わっている必要がある、
	// ここでは最後にSignalしたFenceまで待機して、GPU側のBackBuffer使用が終わるのを待つ。
	m_directCommandQueue.WaitForGPUIdleIfNeeded();

	// フレームリソースにバックバッファ情報を残していてはいけないので全てのフレームリソースに対してリセット処理を行う
	for (const auto& l_frameResource : m_frameResourceList)
	{
		if (!l_frameResource) { continue; }

		const auto& l_commandAllocator = l_frameResource->GetREFDirectCommandAllocator();

		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_commandAllocator, "ダイレクトコマンドアロケータが無効のため、スワップチェインリサイズ前処理に失敗しました。", false);

		// コマンドアロケータをリセット
		// コマンドアロケータは、コマンドリストに記録した命令のメモリを管理するもの
		// GPU待機後なので安全に利用可能
		l_commandAllocator->Reset();
	}

	const auto& l_currentFrameResource = m_currentFrameResource.lock();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_currentFrameResource, "フレームリソースの取得に失敗しており、スワップチェインリサイズ前処理に失敗しました。", false);

	const auto& l_commandAllocator = l_currentFrameResource->GetREFDirectCommandAllocator();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_commandAllocator, "ダイレクトコマンドアロケータが無効になっており、スワップチェインリサイズ前処理に失敗しました。", false);

	// DirectCommandListをリセット
	// これにより、前フレームで記録したBackBufferへのResourceBarrierなどの参照を外す。
	m_directCommandList.Reset(*l_commandAllocator);

	// Resetした直後のコマンドリストは「記録中」の状態になる。
	// このままにすると次のBeginDraw()で再度Reset出来なくなるため、空のままCloseしておく
	m_directCommandList.Close();

	return true;
}