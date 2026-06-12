#include "RenderArea.h"

bool FWK::Graphics::RenderArea::SetupRenderArea(const SwapChain& a_swapChain)
{
	const auto& l_swapChain = a_swapChain.GetREFSwapChain();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_swapChain, "スワップチェインが作成されておらず、ビューポート及びシザー矩形の作成に失敗しました。", false);

	const auto& l_backBufferList = a_swapChain.GetREFBackBufferList();
	
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_backBufferList.empty(), "バックバッファーの要素が空になっており、ビューポート及びシザー矩形の作成に失敗しました。", false);

	// 先頭のバックバッファを取得
	// Viewport生成時に、このバックバッファの幅と高さを利用する
	const auto& l_firstBackBuffer = l_backBufferList.begin()->m_backBufferResource.Get();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_firstBackBuffer, "バックバッファー作成ができておらず、ビューポート及びシザー矩形の作成に失敗しました。", false);

	DXGI_SWAP_CHAIN_DESC1 l_desc = {};

	// スワップチェインからパラメータを取得し縦幅と横幅を取得
	auto l_hr = l_swapChain->GetDesc1(&l_desc);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(FAILED(l_hr), "スワップチェインのパラメーターの取得に失敗しており、ビューポート及びシザー矩形の作成に失敗しました。", false);

	// D3D12_VIEWPORT構造体について説明
	// TopLeftX : 描画開始位置の左端X座標
	// TopLeftY : 描画開始位置の上端Y座標
	// Width    : 描画先の横幅
	// Height   : 描画先の縦幅
	// MinDepth : 深度値の最小値
	// MaxDepth : 深度値の最大値
	// 実際の画面上に画面全体の大きさをどのようにして描画するかを決める設定
	m_viewport = CD3DX12_VIEWPORT{ l_firstBackBuffer };

	// D3D12_RECT構造体について説明
	// left   : 描画可能範囲の左端X座標
	// top    : 描画可能範囲の上端Y座標
	// right  : 描画可能範囲の右端X座標
	// bottom : 描画可能範囲の下端Y座標
	// 実際に描画する範囲を矩形で切り取る設定
	m_scissorRECT = CD3DX12_RECT{ k_defaultScissorRECTLeft,
								  k_defaultScissorRECTTop,
								  static_cast<LONG>(l_desc.Width),
								  static_cast<LONG>(l_desc.Height) };
	
	if (!m_cbSpritePass)
	{
		m_cbSpritePass = std::make_shared<Struct::CBSpritePass>();
	}

	m_cbSpritePass->m_projectionMatrix = TypeAlias::Math::Matrix::CreateOrthographic(m_viewport.Width,
																				     m_viewport.Height,
																					 k_defaultNearClip,
																					 k_defaultFarClip);

	return true;
}

void FWK::Graphics::RenderArea::SyncSpritePassDrawRequest(const RenderGraph& a_renderGraph)
{
	const auto& l_spritePassDrawRequest = a_renderGraph.FindVALDrawRequestPass<SpritePassDrawRequest>().lock();

	if (!l_spritePassDrawRequest) { return; }

	// 定数バッファの変更を反映するために定数バッファデータを送信する
	l_spritePassDrawRequest->SetSourceConstantBuffer(m_cbSpritePass);
}