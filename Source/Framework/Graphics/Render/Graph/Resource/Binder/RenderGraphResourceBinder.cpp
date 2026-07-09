#include "RenderGraphResourceBinder.h"

void FWK::Graphics::RenderGraphResourceBinder::SetupPassRenderTarget(const ResourceContext& a_resourceContext, const RenderGraphPassBase& a_pass, const Renderer& a_renderer) const
{
	// RenderTargetとDepthStencilの両方を書き込むPassなら、
	// OMSetRenderTargetsでRTVとDSVを同時に設定する
	if (SetupPassRenderTargetAndDepthStencil(a_resourceContext, a_pass, a_renderer)) { return; }

	for (const auto& l_resourceAccess : a_pass.GetREFResourceAccessList())
	{
		// レンダーターゲットに書き込まなければcontinue
		if (!IsWriteBackBufferAccess(l_resourceAccess) &&
			!IsWriteRenderTargetPassTextureAccess(l_resourceAccess))
		{
			continue; 
		}

		if (SetupBackBufferRenderTarget(a_resourceContext, a_renderer, l_resourceAccess))			   { continue; }
		if (SetupRenderTargetPassTextureRenderTarget(a_resourceContext, a_renderer, l_resourceAccess)) { continue; }

		FWK_ASSERT_RETURN("ResourceAccessに対応するRenderTargetが存在しないため、Passの描画先設定に失敗しました。");
	}
}

bool FWK::Graphics::RenderGraphResourceBinder::SetupBackBufferRenderTarget(const ResourceContext& a_resourceContext, const Renderer& a_renderer, const Struct::RenderGraphResourceAccess& a_resourceAccess) const
{
	if (!a_resourceAccess.m_isBackBuffer) { return false; }

	const auto& l_swapChain         = a_renderer.GetREFSwapChain        ();
	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();

	const auto  l_backBufferIndex = l_swapChain.FetchVALCurrentBackBufferIndex();
	const auto& l_backBufferList  = l_swapChain.GetREFBackBufferList		  ();

	FWK_ASSERT_RETURN_VALUE_IF(l_backBufferList.empty(),                                        "BackBufferListが空のため、BackBufferの描画先設定に失敗しました。",      false);
	FWK_ASSERT_RETURN_VALUE_IF(l_backBufferIndex >= static_cast<UINT>(l_backBufferList.size()), "BackBufferIndexが範囲外のため、BackBufferの描画先設定に失敗しました。", false);

	const auto& l_backBuffer = l_backBufferList[l_backBufferIndex];

	FWK_ASSERT_RETURN_VALUE_IF(l_backBuffer.m_rtvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "BackBufferのRTVDescriptorIndexが無効のため、BackBufferの描画先設定に失敗しました。", false);

	const auto& l_rtvDescriptorPool = a_resourceContext.GetREFRTVDescriptorPool();
	const auto& l_renderArea        = a_renderer.GetREFRenderArea			   ();

	// ビューポート、シザー矩形のセット、バックバッファをOMにセット
	l_directCommandList.SetupRenderArea  (l_renderArea);
	l_directCommandList.SetupRenderTarget(l_rtvDescriptorPool, l_backBuffer.m_rtvDescriptorIndex);

	return true;
}
bool FWK::Graphics::RenderGraphResourceBinder::SetupRenderTargetPassTextureRenderTarget(const ResourceContext& a_resourceContext, const Renderer& a_renderer, const Struct::RenderGraphResourceAccess& a_resourceAccess) const
{
	if (a_resourceAccess.m_renderTargetType == Enum::RenderGraphRenderTargetType::None) { return false; }

	const auto& l_currentFrameResource = a_renderer.GetREFCurrentFrameResource().lock();

	FWK_ASSERT_RETURN_VALUE_IF(!l_currentFrameResource, "現在のFrameResourceが無効のため、RenderTargetPassTextureの描画先設定に失敗しました。", false);

	const auto& l_renderGraphFrameResource = l_currentFrameResource->GetREFRenderGraphFrameResource();

	// セットするレンダーターゲットパステクスチャを取得
	const auto& l_renderTargetPassTexture  = l_renderGraphFrameResource.FindVALRenderTargetPassTexture(a_resourceAccess.m_renderTargetType).lock();

	if (!l_renderTargetPassTexture) { return false; }

	const auto& l_renderTargetTexture = l_renderTargetPassTexture->GetREFRenderTargetTexture();

	FWK_ASSERT_RETURN_VALUE_IF(l_renderTargetTexture.GetVALRTVDescriptorIndex() == DescriptorHeap::k_invalidDescriptorIndex, "RenderTargetPassTextureのRTVDescriptorIndexが無効のため、描画先設定に失敗しました。", false);

	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList       ();
	const auto& l_rtvDescriptorPool = a_resourceContext.GetREFRTVDescriptorPool();
	const auto& l_renderArea        = a_renderer.GetREFRenderArea			   ();

	// ビューポート、シザー矩形のセット、レンダーターゲットパステクスチャをOMにセット
	l_directCommandList.SetupRenderArea  (l_renderArea);
	l_directCommandList.SetupRenderTarget(l_rtvDescriptorPool, l_renderTargetTexture.GetVALRTVDescriptorIndex());
	
	return true;
}
bool FWK::Graphics::RenderGraphResourceBinder::SetupPassRenderTargetAndDepthStencil(const ResourceContext& a_resourceContext, const RenderGraphPassBase& a_pass, const Renderer& a_renderer) const
{
	bool l_hasRenderTarget = false;
	bool l_hasDepthStencil = false;

	Enum::RenderGraphRenderTargetType l_renderTargetType = Enum::RenderGraphRenderTargetType::None;
	Enum::RenderGraphDepthStencilType l_depthStencilType = Enum::RenderGraphDepthStencilType::None;

	// レンダーターゲットパステクスチャ、デプスステンシルパステクスチャの双方が書き込むかどうかを確認する
	for (const auto& l_resourceAccess : a_pass.GetREFResourceAccessList())
	{
		if (IsWriteRenderTargetPassTextureAccess(l_resourceAccess))
		{
			FWK_ASSERT_RETURN_VALUE_IF(l_hasRenderTarget, "一つのPassに複数のRenderTarget書き込みが指定されています。SetupPassRenderTargetAndDepthStencilの処理に失敗しました。", false);

			l_hasRenderTarget  = true;
			l_renderTargetType = l_resourceAccess.m_renderTargetType;

			continue;
		}

		if (IsWriteDepthStencilPassTextureAccess(l_resourceAccess))
		{
			FWK_ASSERT_RETURN_VALUE_IF(l_hasDepthStencil, "一つのPassに複数のDepthStencil書き込みが指定されています。SetupPassRenderTargetAndDepthStencilの処理に失敗しました。", false);

			l_hasDepthStencil  = true;
			l_depthStencilType = l_resourceAccess.m_depthStencilType;

			continue;
		}
	}

	// RenderTargetとDepthStencilの両方を持つPassだけ、この関数で処理する
	if (!l_hasRenderTarget ||
		!l_hasDepthStencil) 
	{
		return false;
	}

	FWK_ASSERT_RETURN_VALUE_IF(l_renderTargetType == Enum::RenderGraphRenderTargetType::Invalid, "RenderTargetResourceTypeが無効のため、RenderTarget + DepthStencilの描画先設定に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF(l_depthStencilType == Enum::RenderGraphDepthStencilType::Invalid, "DepthStencilResourceTypeが無効のため、RenderTarget + DepthStencilの描画先設定に失敗しました。", false);

	const auto l_currentFrameResource = a_renderer.GetREFCurrentFrameResource().lock();

	FWK_ASSERT_RETURN_VALUE_IF(!l_currentFrameResource, "現在のCurrentFrameResourceが無効のため、RenderTarget + DepthStencilの描画先設定に失敗しました。", false);

	const auto& l_renderGraphFrameResource = l_currentFrameResource->GetREFRenderGraphFrameResource   ();
	const auto& l_renderTargetPassTexture  = l_renderGraphFrameResource.FindVALRenderTargetPassTexture(l_renderTargetType).lock();
	const auto& l_depthStencilPassTexture  = l_renderGraphFrameResource.FindVALDepthStencilPassTexture(l_depthStencilType).lock();

	FWK_ASSERT_RETURN_VALUE_IF(!l_renderTargetPassTexture, "RenderTargetPassTextureが無効のため、RenderTarget + DepthStencilの描画先設定に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF(!l_depthStencilPassTexture, "DepthStencilPassTextureが無効のため、RenderTarget + DepthStencilの描画先設定に失敗しました。", false);

	// レンダーターゲットパステクスチャ、デプスステンシルパステクスチャの両方を取得
	const auto& l_renderTargetTexture = l_renderTargetPassTexture->GetREFRenderTargetTexture();
	const auto& l_depthStencilTexture = l_depthStencilPassTexture->GetREFDepthStencilTexture();

	FWK_ASSERT_RETURN_VALUE_IF(l_renderTargetTexture.GetVALRTVDescriptorIndex() == DescriptorHeap::k_invalidDescriptorIndex, "RenderTargetTextureのRTVDescriptorIndexが無効のため、RenderTarget + DepthStencilの描画先設定に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF(l_depthStencilTexture.GetVALDSVDescriptorIndex() == DescriptorHeap::k_invalidDescriptorIndex, "DepthStencilTextureのDSVDescriptorIndexが無効のため、RenderTarget + DepthStencilの描画先設定に失敗しました。", false);

	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList	   ();
	const auto& l_rtvDescriptorPool = a_resourceContext.GetREFRTVDescriptorPool();
	const auto& l_dsvDescriptorPool = a_resourceContext.GetREFDSVDescriptorPool();
	const auto& l_renderArea		= a_renderer.GetREFRenderArea			   ();

	// ビューポート、シザー矩形のセット
	l_directCommandList.SetupRenderArea(l_renderArea);

	// レンダーターゲットパステクスチャ、デプスステンシルパステクスチャをOMにセット
	l_directCommandList.SetupRenderTargetAndDepthStencil(l_rtvDescriptorPool,
														 l_dsvDescriptorPool,
														 l_renderTargetTexture.GetVALRTVDescriptorIndex(),
														 l_depthStencilTexture.GetVALDSVDescriptorIndex());

	return true;
}

bool FWK::Graphics::RenderGraphResourceBinder::IsWriteBackBufferAccess(const Struct::RenderGraphResourceAccess& a_resourceAccess) const
{
	if (!a_resourceAccess.m_isBackBuffer									 ||
		a_resourceAccess.m_accessType  != Enum::RenderGraphAccessType::Write ||
		a_resourceAccess.m_beforeUsage != Enum::RenderGraphResourceUsage::RenderTarget) 
	{
		return false;
	}

	return true;
}
bool FWK::Graphics::RenderGraphResourceBinder::IsWriteRenderTargetPassTextureAccess(const Struct::RenderGraphResourceAccess& a_resourceAccess) const
{
	if (a_resourceAccess.m_isBackBuffer												   ||
		a_resourceAccess.m_renderTargetType == Enum::RenderGraphRenderTargetType::None ||
		a_resourceAccess.m_accessType       != Enum::RenderGraphAccessType::Write	   ||
		a_resourceAccess.m_beforeUsage      != Enum::RenderGraphResourceUsage::RenderTarget)
	{
		return false; 
	}

	return true;
}
bool FWK::Graphics::RenderGraphResourceBinder::IsWriteDepthStencilPassTextureAccess(const Struct::RenderGraphResourceAccess& a_resourceAccess) const
{
	if (a_resourceAccess.m_depthStencilType == Enum::RenderGraphDepthStencilType::None  ||
		a_resourceAccess.m_accessType       != Enum::RenderGraphAccessType::Write       ||
		a_resourceAccess.m_beforeUsage      != Enum::RenderGraphResourceUsage::DepthWrite)
	{
		return false;
	}

	return true;
}