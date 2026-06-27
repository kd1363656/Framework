#pragma once

namespace FWK::Struct
{
	struct RenderGraphResourceAccess final 
	{
		// 今からアクセスするリソースがバックバッファなのか、
		// レンダーターゲットテクスチャなのか、デプスステンシルテクスチャなのかを決めるフラグ
		// アクセスするリソースがバックバッファとレンダーターゲットテクスチャといった不正状態を作れてしまうので
		// リストに追加するときにバックバッファを使うならレンダーターゲットテクスチャ、デプスステンシルテクスチャは
		// 使わないようにする必要がある
		bool							  m_isBackBuffer     = false;
		Enum::RenderGraphRenderTargetType m_renderTargetType = Enum::RenderGraphRenderTargetType::None;
		Enum::RenderGraphDepthStencilType m_depthStencilType = Enum::RenderGraphDepthStencilType::None;
		
		Enum::RenderGraphAccessType    m_accessType  = Enum::RenderGraphAccessType::Invalid;
		Enum::RenderGraphResourceUsage m_beforeUsage = Enum::RenderGraphResourceUsage::Invalid;
		Enum::RenderGraphResourceUsage m_afterUsage  = Enum::RenderGraphResourceUsage::None;
	};
}