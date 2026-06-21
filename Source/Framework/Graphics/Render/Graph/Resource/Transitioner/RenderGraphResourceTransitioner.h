#pragma once

namespace FWK::Graphics
{
	class RenderGraphResourceTransitioner final
	{
	public:

		 RenderGraphResourceTransitioner() = default;
		~RenderGraphResourceTransitioner() = default;

		void TransitionPassResourceBefore(const RenderGraphPassBase& a_pass,              Renderer&             a_renderer)										const;
		void TransitionPassResourceAfter (const RenderGraphPassBase& a_pass,              Renderer&             a_renderer)										const;
		void TransitionBackBufferResource(const DirectCommandList&   a_directCommandList, D3D12_RESOURCE_STATES a_afterState, Struct::BackBuffer& a_backBuffer) const;

	private:

		bool TransitionBackBufferResource			  (const Struct::RenderGraphResourceAccess& a_resourceAccess, const Enum::RenderGraphResourceUsage a_usage,		  Renderer& a_renderer) const;
		bool TransitionRenderTargetPassTextureResource(const Struct::RenderGraphResourceAccess& a_resourceAccess, const Enum::RenderGraphResourceUsage a_usage, const Renderer& a_renderer) const;
		bool TransitionDepthStencilPassTextureResource(const Struct::RenderGraphResourceAccess& a_resourceAccess, const Enum::RenderGraphResourceUsage a_usage, const Renderer& a_renderer) const;
		
		D3D12_RESOURCE_STATES ConvertVALD3D12ResourceState(const Enum::RenderGraphResourceUsage a_usage) const;
	};
}