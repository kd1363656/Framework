#include "RenderGraphResourceTransitioner.h"

void FWK::Graphics::RenderGraphResourceTransitioner::TransitionPassResourceBefore(const RenderGraphPassBase& a_pass, Renderer& a_renderer) const
{
	for (const auto& l_resourceAccess : a_pass.GetREFResourceAccessList())
	{
		const auto l_beforeUsage = l_resourceAccess.m_beforeUsage;

		if (TransitionBackBufferResource(l_resourceAccess, l_beforeUsage,  a_renderer))			    { continue; }
		if (TransitionRenderTargetPassTextureResource(l_resourceAccess, l_beforeUsage, a_renderer)) { continue; }
		if (TransitionDepthStencilPassTextureResource(l_resourceAccess, l_beforeUsage, a_renderer)) { continue; }

		FWK_ASSERT_RETURN("RenderGraphResourceAccessに対応するリソースが存在しないため、Pass実行前の自動リソース遷移に失敗しました。");
	}
}
void FWK::Graphics::RenderGraphResourceTransitioner::TransitionPassResourceAfter(const RenderGraphPassBase& a_pass, Renderer& a_renderer) const
{
	for (const auto& l_resourceAccess : a_pass.GetREFResourceAccessList())
	{
		const auto l_afterUsage = l_resourceAccess.m_afterUsage;

		// 何も遷移する必要がなければ線を実行しない
		if (l_afterUsage == Enum::RenderGraphResourceUsage::None) { continue; }

		if (TransitionBackBufferResource(l_resourceAccess, l_afterUsage,  a_renderer))			   { continue; }
		if (TransitionRenderTargetPassTextureResource(l_resourceAccess, l_afterUsage, a_renderer)) { continue; }
		if (TransitionDepthStencilPassTextureResource(l_resourceAccess, l_afterUsage, a_renderer)) { continue; }

		FWK_ASSERT_RETURN("RenderGraphResourceAccessに対応するリソースが存在しないため、Pass実行後の自動リソース遷移に失敗しました。");
	}
}
void FWK::Graphics::RenderGraphResourceTransitioner::TransitionBackBufferResource(const DirectCommandList& a_directCommandList, D3D12_RESOURCE_STATES a_afterState, Converter::SwapChainJsonConverter::BackBuffer& a_backBuffer) const
{
	FWK_ASSERT_RETURN_IF(!a_backBuffer.m_backBufferResource, "バックバッファリソースが無効になっており、バックバッファリソースの状態遷移に失敗しました。");

	// 既に目的のStateならBarrierは不要
	if (a_backBuffer.m_currentResourceState == a_afterState) { return; }

	a_directCommandList.TransitionResourceBarrier(a_backBuffer.m_backBufferResource, a_backBuffer.m_currentResourceState, a_afterState);

	// 状態遷移後の状態を格納
	a_backBuffer.m_currentResourceState = a_afterState;
}

bool FWK::Graphics::RenderGraphResourceTransitioner::TransitionBackBufferResource(const Struct::RenderGraphResourceAccess& a_resourceAccess, const Enum::RenderGraphResourceUsage a_usage, Renderer& a_renderer) const
{
	if (!a_resourceAccess.m_isBackBuffer) { return false; }

	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();
	const auto  l_afterState        = ConvertVALD3D12ResourceState	    (a_usage);
		  auto& l_swapChain			= a_renderer.GetMutableREFSwapChain ();

	const auto  l_backBufferIndex = l_swapChain.FetchVALCurrentBackBufferIndex();
		  auto& l_backBufferList  = l_swapChain.GetMutableREFBackBufferList   ();

	FWK_ASSERT_RETURN_VALUE_IF(l_backBufferList.empty(),										    "BackBufferListが空のため、BackBufferの自動リソース遷移に失敗しました。",      true);
	FWK_ASSERT_RETURN_VALUE_IF(l_backBufferIndex >= static_cast<UINT>(l_backBufferList.size()), "BackBufferIndexが範囲外のため、BackBufferの自動リソース遷移に失敗しました。", true);

	auto& l_backBuffer = l_backBufferList[l_backBufferIndex];

	// バックバッファのリソース状態の遷移
	TransitionBackBufferResource(l_directCommandList, l_afterState, l_backBuffer);

	return true;
}
bool FWK::Graphics::RenderGraphResourceTransitioner::TransitionRenderTargetPassTextureResource(const Struct::RenderGraphResourceAccess& a_resourceAccess, const Enum::RenderGraphResourceUsage a_usage, const Renderer& a_renderer) const
{
	if (a_resourceAccess.m_renderTargetType == Enum::RenderGraphRenderTargetType::None) { return false; }

	const auto& l_currentFrameResource = a_renderer.GetREFCurrentFrameResource().lock();

	FWK_ASSERT_RETURN_VALUE_IF(!l_currentFrameResource, "現在のFrameResourceが無効のため、RenderTargetPassTextureの自動リソース遷移に失敗しました。", false);

	// 現在のフレームリソースからレンダーターゲットパステクスチャを取得
	const auto& l_renderGraphFrameResource = l_currentFrameResource->GetREFRenderGraphFrameResource   ();
	const auto& l_renderTargetPassTexture  = l_renderGraphFrameResource.FindVALRenderTargetPassTexture(a_resourceAccess.m_renderTargetType).lock();

	if (!l_renderTargetPassTexture) { return false; }

	auto& l_renderTargetTexture = l_renderTargetPassTexture->GetMutableREFRenderTargetTexture();

	// リソース遷移の対象になるGPUResourceを取得
	const auto& l_gpuResource = l_renderTargetTexture.GetREFGPUResource();

	FWK_ASSERT_RETURN_VALUE_IF(!l_gpuResource.m_resource, "RenderTargetPassTextureのGPUResourceが無効のため、自動リソース遷移に失敗しました。", false);

	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();

	const auto l_beforeState = l_renderTargetTexture.GetVALCurrentResourceState();
	const auto l_afterState  = ConvertVALD3D12ResourceState					  (a_usage);

	// 既に必要な状態ならResourceBarrierは不要
	if (l_beforeState == l_afterState) { return true; }

	// リソース状態の遷移
	l_directCommandList.TransitionResourceBarrier(l_gpuResource.m_resource, l_beforeState, l_afterState);

	// RenderGraph側でRenderTargetTextureのCPU側Stateを更新する
	l_renderTargetTexture.SetCurrentResourceState(l_afterState);

	return true;
}
bool FWK::Graphics::RenderGraphResourceTransitioner::TransitionDepthStencilPassTextureResource(const Struct::RenderGraphResourceAccess& a_resourceAccess, const Enum::RenderGraphResourceUsage a_usage, const Renderer& a_renderer) const
{
	if (a_resourceAccess.m_depthStencilType == Enum::RenderGraphDepthStencilType::None) { return false; }

	const auto l_currentFrameResource = a_renderer.GetREFCurrentFrameResource().lock();

	FWK_ASSERT_RETURN_VALUE_IF(!l_currentFrameResource, "現在のFrameResourceが無効のため、DepthStencilPassTextureの自動リソース遷移に失敗しました。", false);

	const auto& l_renderGraphFrameResource = l_currentFrameResource->GetREFRenderGraphFrameResource	  ();
	const auto& l_depthStencilPassTexture  = l_renderGraphFrameResource.FindVALDepthStencilPassTexture(a_resourceAccess.m_depthStencilType).lock();

	if (!l_depthStencilPassTexture) { return false; }

	auto& l_depthStencilTexture = l_depthStencilPassTexture->GetMutableREFDepthStencilTexture();

	const auto& l_gpuResource = l_depthStencilTexture.GetREFGPUResource();

	FWK_ASSERT_RETURN_VALUE_IF(!l_gpuResource.m_resource, "DepthStencilPassTextureのGPUResourceが無効のため、自動リソース遷移に失敗しました。", false);

	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();

	const auto l_beforeState = l_depthStencilTexture.GetVALCurrentResourceState();
	const auto l_afterState  = ConvertVALD3D12ResourceState					   (a_usage);

	if (l_beforeState == l_afterState) { return true; }

	// リソースの遷移を行う
	l_directCommandList.TransitionResourceBarrier(l_gpuResource.m_resource, l_beforeState, l_afterState);
	l_depthStencilTexture.SetCurrentResourceState(l_afterState);

	return true;
}

D3D12_RESOURCE_STATES FWK::Graphics::RenderGraphResourceTransitioner::ConvertVALD3D12ResourceState(const Enum::RenderGraphResourceUsage a_usage) const
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

		case Enum::RenderGraphResourceUsage::DepthWrite:
		{
			return D3D12_RESOURCE_STATE_DEPTH_WRITE;
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