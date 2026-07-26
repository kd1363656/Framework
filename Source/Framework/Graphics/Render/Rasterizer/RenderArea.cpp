#include "RenderArea.h"

bool FWK::Graphics::RenderArea::Setup(const UINT a_width, const UINT a_height)
{
	FWK_ASSERT_RETURN_VALUE_IF(!Utility::IsValidTextureSize(a_width, a_height), "RenderAreaへ指定されたWidthまたはHeightが無効です。", false);

	// D3D12_VIEWPORT構造体について説明
	// TopLeftX : 描画開始位置の左端X座標
	// TopLeftY : 描画開始位置の上端Y座標
	// Width    : 描画先の横幅
	// Height   : 描画先の縦幅
	// MinDepth : 深度値の最小値
	// MaxDepth : 深度値の最大値
	// 実際の画面上に画面全体の大きさをどのようにして描画するかを決める設定
	const auto& l_viewport = D3D12_VIEWPORT{ k_defaultViewportTopLeftX,
	                                         k_defaultViewportTopLeftY,
	                                         static_cast<float>(a_width),
	                                         static_cast<float>(a_height),
	                                         Constant::k_renderAreaMINViewportDepth,
	                                         Constant::k_renderAreaMAXViewportDepth };

	// D3D12_RECT構造体について説明
	// left   : 描画可能範囲の左端X座標
	// top    : 描画可能範囲の上端Y座標
	// right  : 描画可能範囲の右端X座標
	// bottom : 描画可能範囲の下端Y座標
	// 実際に描画する範囲を矩形で切り取る設定
	const auto& l_scissorRECT = D3D12_RECT{ k_defaultScissorRECTLeft,
								            k_defaultScissorRECTTop,
								            static_cast<LONG>(a_width),
								            static_cast<LONG>(a_height) };

	return Setup(l_viewport, l_scissorRECT);
}

bool FWK::Graphics::RenderArea::Setup(const D3D12_VIEWPORT& a_viewport, const D3D12_RECT& a_scissorRECT)
{
	FWK_ASSERT_RETURN_VALUE_IF(a_viewport.Width  <= k_invalidViewportSize ||
		                       a_viewport.Height <= k_invalidViewportSize,
		                       "RenderAreaへ指定されたViewportのWidthまたはHeightが無効です。",
		                       false);

	FWK_ASSERT_RETURN_VALUE_IF(a_viewport.MinDepth <  Constant::k_renderAreaMINViewportDepth ||
		                       a_viewport.MaxDepth >  Constant::k_renderAreaMAXViewportDepth ||
		                       a_viewport.MinDepth >= a_viewport.MaxDepth,
		                       "RenderAreaへ指定されたViewportのDepth範囲が無効です。",
		                       false);

	FWK_ASSERT_RETURN_VALUE_IF(a_scissorRECT.right  <= a_scissorRECT.left ||
		                       a_scissorRECT.bottom <= a_scissorRECT.top,
		                       "RenderAreaへ指定されたScissorRECTの範囲が無効です。",
		                       false);

	m_viewport    = a_viewport;
	m_scissorRECT = a_scissorRECT;

	return true;
}