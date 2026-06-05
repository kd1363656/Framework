#include "GraphicsManager.h"

void FWK::Graphics::GraphicsManager::INIT()
{
#if defined(_DEBUG)
	FWK_ASSERT_RETURN_IF_FAILED(!EnableDebugLayer(), "デバッグレイヤーの有効化に失敗しました。");
#endif
}
void FWK::Graphics::GraphicsManager::LoadCONFIG()
{
	const auto& l_rootJson = Utility::LoadJsonFile(k_configFileIOPath);

	if (l_rootJson.is_null()) { return; }

	m_graphicsManagerJsonConverter.Deserialize(l_rootJson, *this);
}
bool FWK::Graphics::GraphicsManager::PostLoadCONFIG(const Window& a_window)
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_factory.Create(),							"ファクトリーの作成に失敗しました。",					        false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_device.Create(m_factory),					"デバイスの作成処理に失敗しました。",					        false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_resourceContext.PostDeserialize(m_device), "リソースコンテキストのデシリアライズ後の処理に失敗しました。", false);

	auto& l_rtvDescriptorPool = m_resourceContext.GetMutableREFRTVDescriptorPool();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_renderer.PostDeserialize(m_device, 
									  a_window,
									  m_factory,
									  l_rtvDescriptorPool),
									  "レンダラーのデシリアライズ後の処理に失敗しました。",
									  false);

	return true;
}

void FWK::Graphics::GraphicsManager::BeginFrame()
{
	auto& l_rtvDescriptorPool = m_resourceContext.GetMutableREFRTVDescriptorPool();

	m_renderer.BeginFrame(l_rtvDescriptorPool);
}
void FWK::Graphics::GraphicsManager::EndFrame()
{
	m_renderer.EndFrame();
}

void FWK::Graphics::GraphicsManager::SaveCONFIG() const
{
	const auto& l_rootJson = m_graphicsManagerJsonConverter.Serialize(*this);

	Utility::SaveJsonFile(l_rootJson, k_configFileIOPath);
}

void FWK::Graphics::GraphicsManager::ProcessWindowResizeRequest(const Struct::WindowResizeRequest& a_windowResizeRequest)
{
	// window側でサイズ変更が起きていない場合は、何もしない
	// 最小化中はクライアント領域が0になることがある
	// この状態でSwapChainやRenderTargetを作り直すと、0サイズのGPUリソース作成になって失敗してしまう。
	if (!a_windowResizeRequest.m_isRequested ||
		a_windowResizeRequest.m_isMinimized) 
	{
		return; 
	}

	// ResizeBuffers()の前に、Rendererが待つDirectCommandList側のBackBuffer参照を外す、
	// 前フレームのResourceBarrierなどがコマンドリスト内部に残っていると、
	// SwapChain::ReleaseBackBufferList()でComPtrをResetしてもResizeBuffers()が失敗することがある
	m_renderer.Resize(m_device, a_windowResizeRequest.m_clientSize, m_resourceContext.GetMutableREFRTVDescriptorPool());
}

#if defined(_DEBUG)
bool FWK::Graphics::GraphicsManager::EnableDebugLayer() const
{
	TypeAlias::ComPtr<ID3D12Debug5> l_debug = nullptr;

	// デバッグ機能を有効化するためのインターフェースを取得する関数
	// D3D12GetDebugInterface(受け取りたいCOMインターフェース型のID、
	//					      作成結果のポインタを書き込むアドレス);
	auto l_hr = D3D12GetDebugInterface(IID_PPV_ARGS(l_debug.ReleaseAndGetAddressOf()));

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(FAILED(l_hr), "デバッグレイヤーの有効化に失敗しました。", false);

	l_debug->EnableDebugLayer();

	return true;
}
#endif