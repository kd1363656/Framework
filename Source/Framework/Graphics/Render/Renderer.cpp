#include "Renderer.h"

void FWK::Graphics::Renderer::INIT()
{
	if (!m_cbSpritePass) {return; }
	
	m_cbSpritePass = std::make_shared<Struct::CBSpritePass>();
}

void FWK::Graphics::Renderer::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}
bool FWK::Graphics::Renderer::PostDeserialize(const Device&			    a_device, 
											  const Window&			    a_window, 
											  const Factory&		    a_factory,
											  const Window::ClientSize& a_clientSize,
													ResourceContext&    a_resourceContext)
{
	// フレームリソースがないとコマンドアロケーターを使えないため"return"
	FWK_ASSERT_RETURN_VALUE_IF(m_frameResourceList.empty(), "フレームリソースリストが空になっており、フレームリソース作成処理に失敗しました。", false);

	const auto& l_gpuMemoryAllocator = a_resourceContext.GetREFGPUMemoryAllocator();
	const auto& l_shaderCompiler     = a_resourceContext.GetREFShaderCompiler    ();

	auto& l_rtvDescriptorPool       = a_resourceContext.GetMutableREFRTVDescriptorPool      ();
	auto& l_cbvSRVUAVDescriptorPool = a_resourceContext.GetMutableREFCBVSRVUAVDescriptorPool();
	auto& l_dsvDescriptorPool       = a_resourceContext.GetMutableREFDSVDescriptorPool      ();

	for (const auto& l_frameResource : m_frameResourceList)
	{
		if (!l_frameResource) { continue; }

		FWK_ASSERT_RETURN_VALUE_IF(!l_frameResource->Create(a_device,
															l_gpuMemoryAllocator,
															a_clientSize,
															a_resourceContext), 
															"フレームリソースの作成処理に失敗しました。", 
															false);
	}

	// ShadowContextが所有するShadow用リソースを作成する
	FWK_ASSERT_RETURN_VALUE_IF(!m_shadowContext.Create(a_device,
		                                               l_gpuMemoryAllocator,
		                                               l_dsvDescriptorPool,
		                                               l_cbvSRVUAVDescriptorPool),
		                                               "ShadowContextの作成処理に失敗しました。",
		                                               false);

	// ダイレクトコマンドキュー、リスト、コンピュートキュー、リストの作成処理
	FWK_ASSERT_RETURN_VALUE_IF(!m_directCommandQueue.Create(a_device),  "ダイレクトコマンドキューの作成処理に失敗しました。",   false);
	FWK_ASSERT_RETURN_VALUE_IF(!m_directCommandList.Create(a_device),   "ダイレクトコマンドリストの作成処理に失敗しました。",   false);
	FWK_ASSERT_RETURN_VALUE_IF(!m_computeCommandQueue.Create(a_device), "コンピュートコマンドキューの作成処理に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF(!m_computeCommandList.Create(a_device),  "コンピュートコマンドリストの作成処理に失敗しました。", false);

	FWK_ASSERT_RETURN_VALUE_IF(!m_swapChain.Create(a_window,
												   a_device,
												   a_factory, 
												   m_directCommandQueue, 
												   l_rtvDescriptorPool), 
												   "ダイレクトコマンドリストの作成処理に失敗しました。",
												   false);

	// ルートシグネチャの作成処理
	for (const auto& [l_type, l_rootSignature] : m_rootSignatureMap)
	{
		FWK_ASSERT_RETURN_VALUE_IF(!l_rootSignature,                   "RootSignatureが無効のため、RootSignatureの作成に失敗しました。", false);
		FWK_ASSERT_RETURN_VALUE_IF(!l_rootSignature->Create(a_device), "RootSignatureの作成処理に失敗しました。",                        false);
	}

	// パイプラインステートの作成処理
	for (const auto& [l_type, l_pipelineState] : m_pipelineStateMap)
	{
		FWK_ASSERT_RETURN_VALUE_IF(!l_pipelineState,                                            "PipelineStateが無効のため、PipelineStateの作成に失敗しました。", false);
		FWK_ASSERT_RETURN_VALUE_IF(!l_pipelineState->Create(a_device, l_shaderCompiler, *this), "PipelineStateの作成処理に失敗しました。",                        false);
	}

	// 画面解像度に合ったビューポート、シザー矩形を作成する
	FWK_ASSERT_RETURN_VALUE_IF(!SetupScreenRenderArea(a_clientSize), "レンダーエリア作成に失敗しました。", false);

	// ALT + ENTERキーで排他フルスクリーン設定が反映されないようにする
	m_swapChain.PostCreateSetup(a_window.GetREFHWND(), a_factory);

	// 定数バッファを各パスに送信
	SyncSpritePassDrawRequest();

	// ShadowContextで定数バッファポインタを各パスにセットする
	m_shadowContext.Setup(*this);

	// レンダーパスの依存順序の解決を行う
	m_renderGraph.Compile();

	// 最初に使うフレームリソースをキャッシュ
	m_currentFrameResource = m_frameResourceList[k_initialFrameResourceIndex];

	return true;
}

void FWK::Graphics::Renderer::BeginFrame(const ResourceContext& a_resourceContext)
{
	// 現在のフレームリソースの定数バッファのインデックスの初期化
	const auto& l_currentFrameResource = m_currentFrameResource.lock();

	FWK_ASSERT_RETURN_IF(!l_currentFrameResource, "フレームリソースの取得に失敗しており、描画開始処理に失敗しました。");

	ResetCommandObjects(*l_currentFrameResource);

	// GPUがこのフレームリソースを使い終わってから、
	// ConstantBufferUploaderの書き込みインデックスを先頭に戻す
	l_currentFrameResource->BeginFrame();

	// リソース遷移の実行
	m_renderGraph.BeginFrame(a_resourceContext, *this);
}
void FWK::Graphics::Renderer::Execute(const ResourceContext& a_resourceContext)
{
	const auto& l_currentFrameResource = m_currentFrameResource.lock();

	FWK_ASSERT_RETURN_IF(!l_currentFrameResource, "フレームリソースの取得に失敗しており、描画開始処理に失敗しました。");

	m_renderGraph.Execute(a_resourceContext, *this);
}
void FWK::Graphics::Renderer::EndFrame()
{
	const auto& l_currentFrameResource = m_currentFrameResource.lock();

	FWK_ASSERT_RETURN_IF(!l_currentFrameResource, "フレームリソースの取得に失敗しており、描画終了処理に失敗しました。");

	const auto& l_directCommandAllocator  = l_currentFrameResource->GetREFDirectCommandAllocator ();
	const auto& l_computeCommandAllocator = l_currentFrameResource->GetREFComputeCommandAllocator();

	FWK_ASSERT_RETURN_IF(!l_directCommandAllocator,  "ダイレクトコマンドアロケータが無効になっており、描画終了処理に失敗しました。");
	FWK_ASSERT_RETURN_IF(!l_computeCommandAllocator, "コンピュートコマンドアロケータが無効になっており、描画終了処理に失敗しました。");

	// BackBufferをREBDER_TARGETからPRESENTへ遷移する命令を、
	// DirectCommandListへ記録する
	m_renderGraph.EndFrame(*this);

	// 各CommandListへの命令記録を終了する
	m_computeCommandList.Close();
	m_directCommandList.Close ();

	// AnimationComputeなどのGPU計算を先に実行する
	m_computeCommandQueue.ExecuteCommandLists(m_computeCommandList);

	// ComputeCommandAllocatorが安全に再利用できるように、
	// 今回のCompute処理完了地点へFenceSignalを登録する
	m_computeCommandQueue.SignalAndTrackAllocator(*l_computeCommandAllocator);

	const auto& l_computeFenceValue = m_computeCommandQueue.FetchREFLastSignaledFenceValue();

	FWK_ASSERT_RETURN_IF(l_computeFenceValue == Fence::k_unusedFenceValue, "ComputeQueueのFenceSignalに失敗したため、DirectQueueとの同期に失敗しました。");

	// DirectQueueへGPUWaitを登録する
	// CPUスレッドは停止せず、
	// GPU上でComputeQueueがBoneMatrixを書き終わってから
	// DirectQueueの描画処理を開始する
	FWK_ASSERT_RETURN_IF(!m_directCommandQueue.Wait(m_computeCommandQueue, l_computeFenceValue), "ComputeQueueとDirectQueueの同期に失敗しました。");

	// ComputeShaderが作成したBoneMatrixを使用して描画する
	m_directCommandQueue.ExecuteCommandLists(m_directCommandList);

	// DirectCommandAllocatorの再利用に必要なFence値を記録する
	m_directCommandQueue.SignalAndTrackAllocator (*l_directCommandAllocator);

	// BackBufferを画面へ表示する
	m_swapChain.Present();

	// 次のフレームで使用するフレームリソースを決める
	DecideNextFrameUseFrameResource();
}

nlohmann::json FWK::Graphics::Renderer::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

void FWK::Graphics::Renderer::Resize(const Device& a_device, const Window::ClientSize& a_clientSize, ResourceContext& a_resourceContext)
{
	// スワップチェインのリサイズ前にGPUとの同期をとるなど必要な処理を行う
	PrepareForSwapChainResize();

	const auto& l_gpuMemoryAllocator     = a_resourceContext.GetREFGPUMemoryAllocator      ();
	const auto& l_resourceReleaseContext = a_resourceContext.GetREFResourceReleaseContext  ();
		  auto& l_rtvDescriptorPool      = a_resourceContext.GetMutableREFRTVDescriptorPool();
		  
	// バックバッファのリサイズを行う
	FWK_ASSERT_RETURN_IF(!m_swapChain.Resize(a_device,
											 l_resourceReleaseContext, 
											 a_clientSize, 
											 l_rtvDescriptorPool), 
											 "バックバッファのリサイズ処理に失敗しており、リサイズ処理に失敗しました");

	// リサイズ後のClientSizeから、
	// 画面用Viewport、ScissorRECT、Sprite用正射影行列を更新する。
	FWK_ASSERT_RETURN_IF(!SetupScreenRenderArea(a_clientSize), "リサイズ後のScreenRenderArea設定処理に失敗しました。");

	const auto& l_retiredFenceValue = m_directCommandQueue.FetchREFLastSignaledFenceValue();

	for (const auto& l_frameResource : m_frameResourceList)
	{
		if (!l_frameResource) { continue; }

		FWK_ASSERT_RETURN_IF(!l_frameResource->Resize(a_device,
													  l_gpuMemoryAllocator,
													  a_clientSize,
													  l_retiredFenceValue,
													  a_resourceContext),
													  "フレームリソースのリサイズ処理に失敗しており、リサイズ処理に失敗しました。");
	}
}

void FWK::Graphics::Renderer::AddFrameResource(const std::shared_ptr<FrameResource>& a_frameResource)
{
	FWK_ASSERT_RETURN_IF(!a_frameResource, "FrameResourceが無効のため、FrameResourceListへの登録に失敗しました。");

	m_frameResourceList.emplace_back(a_frameResource);
}
void FWK::Graphics::Renderer::AddRootSignature(const std::shared_ptr<RootSignature>& a_rootSignature, const Enum::RootSignatureType a_rootSignatureType)
{
	FWK_ASSERT_RETURN_IF(!a_rootSignature,										  "RootSignatureが無効のため、RootSignatureMapへの登録に失敗しました。");
	FWK_ASSERT_RETURN_IF(a_rootSignatureType == Enum::RootSignatureType::Invalid, "RootSignatureが無効な種類のため、RootSignatureMapへの登録に失敗しました。");

	m_rootSignatureMap.try_emplace(a_rootSignatureType, a_rootSignature);	
}
void FWK::Graphics::Renderer::AddPipelineState(const std::shared_ptr<PipelineStateBase>& a_pipelineState, const Enum::PipelineStateType a_pipelineStateType)
{
	FWK_ASSERT_RETURN_IF(!a_pipelineState,										  "PipelineStateが無効のため、PipelineStateMapへの登録に失敗しました。");
	FWK_ASSERT_RETURN_IF(a_pipelineStateType == Enum::PipelineStateType::Invalid, "PipelineStateが無効な種類のため、PipelineStateMapへの登録に失敗しました。");

	m_pipelineStateMap.try_emplace(a_pipelineStateType, a_pipelineState);
}

std::weak_ptr<FWK::Graphics::RootSignature> FWK::Graphics::Renderer::FindVALRootSignature(const Enum::RootSignatureType a_rootSignatureType) const
{
	const auto& l_itr = m_rootSignatureMap.find(a_rootSignatureType);

	if (l_itr == m_rootSignatureMap.end()) { return {}; }

	return l_itr->second;
}

bool FWK::Graphics::Renderer::SetupScreenRenderArea(const Window::ClientSize& a_clientSize)
{
	FWK_ASSERT_RETURN_VALUE_IF(!m_cbSpritePass, "SpritePass用ConstantBufferが作成されておらず、ScreenRenderAreaの設定処理に失敗しました。", false);

	// WindowのClientSizeから、
	// 画面描画専用のViewportとScissorRectを設定する
	// RenderArea自身はWindowやSwapChainを知らない
	FWK_ASSERT_RETURN_VALUE_IF(!m_screenRenderArea.Setup(a_clientSize.m_width, a_clientSize.m_height), "WindowのClient Sizeを使用したScreenRenderAreaの設定処理に失敗しました。", false);

	const auto& l_viewport = m_screenRenderArea.GetREFViewport();

	// Sprite Passは画面のピクセル座標を使用するため、
	// ScreenRenderAreaと同じ幅と高さから正射影行列を作成する。
	//
	// この定数バッファはShadow用RenderAreaとは関係しないため、
	// Rendererが所有して更新する。
	m_cbSpritePass->m_projectionMatrix = TypeAlias::Math::Matrix::CreateOrthographic(l_viewport.Width,
			                                                                         l_viewport.Height,
			                                                                         Constant::k_renderAreaMINViewportDepth,
			                                                                         Constant::k_renderAreaMAXViewportDepth);

	return true;
}

void FWK::Graphics::Renderer::ResetCommandObjects(const FrameResource& a_frameResource)
{
	const auto& l_directCommandAllocator  = a_frameResource.GetREFDirectCommandAllocator ();

	FWK_ASSERT_RETURN_IF(!l_directCommandAllocator, "ダイレクトコマンドアロケータが無効になっており、描画開始処理に失敗しました。");

	const auto& l_computeCommandAllocator = a_frameResource.GetREFComputeCommandAllocator();

	FWK_ASSERT_RETURN_IF(!l_computeCommandAllocator, "コンピュートコマンドアロケータが無効になっており、描画開始処理に失敗しました。");

	// コマンドアロケータからGPU処理が終わっているかどうかを確かめGPUの処理が終わっていればWait
	m_directCommandQueue.EnsureAllocatorAvailable (*l_directCommandAllocator);
	m_computeCommandQueue.EnsureAllocatorAvailable(*l_computeCommandAllocator);

	// GPU同期処理が終わってからコマンドリスト、アロケータをリセット
	l_directCommandAllocator->Reset ();
	l_computeCommandAllocator->Reset();

	m_directCommandList.Reset (*l_directCommandAllocator);
	m_computeCommandList.Reset(*l_computeCommandAllocator);
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

		FWK_ASSERT_RETURN_VALUE_IF(!l_commandAllocator, "ダイレクトコマンドアロケータが無効のため、スワップチェインリサイズ前処理に失敗しました。", false);

		// コマンドアロケータをリセット
		// コマンドアロケータは、コマンドリストに記録した命令のメモリを管理するもの
		// GPU待機後なので安全に利用可能
		l_commandAllocator->Reset();
	}

	const auto& l_currentFrameResource = m_currentFrameResource.lock();

	FWK_ASSERT_RETURN_VALUE_IF(!l_currentFrameResource, "フレームリソースの取得に失敗しており、スワップチェインリサイズ前処理に失敗しました。", false);

	const auto& l_commandAllocator = l_currentFrameResource->GetREFDirectCommandAllocator();

	FWK_ASSERT_RETURN_VALUE_IF(!l_commandAllocator, "ダイレクトコマンドアロケータが無効になっており、スワップチェインリサイズ前処理に失敗しました。", false);

	// DirectCommandListをリセット
	// これにより、前フレームで記録したBackBufferへのResourceBarrierなどの参照を外す。
	m_directCommandList.Reset(*l_commandAllocator);

	// Resetした直後のコマンドリストは「記録中」の状態になる。
	// このままにすると次のBeginDraw()で再度Reset出来なくなるため、空のままCloseしておく
	m_directCommandList.Close();

	return true;
}

void FWK::Graphics::Renderer::SyncSpritePassDrawRequest()
{
	const auto& l_spriteScreenPassDrawRequest = m_renderGraph.FindVALDrawRequestPass<SpriteScreenPassDrawRequest>().lock();

	if (!l_spriteScreenPassDrawRequest) { return; }

	// 定数バッファの変更を反映するために定数バッファデータを送信する
	l_spriteScreenPassDrawRequest->SetSourceConstantBuffer(m_cbSpritePass);
}