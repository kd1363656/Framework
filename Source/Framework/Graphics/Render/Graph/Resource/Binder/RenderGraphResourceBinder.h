#pragma once

namespace FWK::Graphics
{
	class RenderGraphResourceBinder final
	{
	public:

		 RenderGraphResourceBinder() = default;
		~RenderGraphResourceBinder() = default;

		void SetupPassRenderTarget(const ResourceContext& a_resourceContext, const RenderGraphPassBase& a_pass, const Renderer& a_renderer) const;

	private:

		bool SetupBackBufferRenderTarget             (const ResourceContext& a_resourceContext, const Renderer&			   a_renderer, const Struct::RenderGraphResourceAccess& a_resourceAccess) const;
		bool SetupRenderTargetPassTextureRenderTarget(const ResourceContext& a_resourceContext, const Renderer&			   a_renderer, const Struct::RenderGraphResourceAccess& a_resourceAccess) const;
		bool SetupPassRenderTargetAndDepthStencil	 (const ResourceContext& a_resourceContext, const RenderGraphPassBase& a_pass,	   const Renderer&						    a_renderer)       const;

		bool IsWriteBackBufferAccess			     (const Struct::RenderGraphResourceAccess& a_resourceAccess) const;
		bool IsWriteRenderTargetPassTextureAccess(const Struct::RenderGraphResourceAccess& a_resourceAccess) const;
		bool IsWriteDepthStencilPassTextureAccess(const Struct::RenderGraphResourceAccess& a_resourceAccess) const;
	};
}