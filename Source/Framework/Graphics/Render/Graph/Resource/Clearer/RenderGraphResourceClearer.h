#pragma once

namespace FWK::Graphics
{
	class RenderGraphResourceClearer final
	{
	public:

		 RenderGraphResourceClearer() = default;
		~RenderGraphResourceClearer() = default;

		void ClearCurrentFramePassTextureList(const ResourceContext& a_resourceContext, const Renderer& a_renderer) const;

	private:

		void ClearCurrentFrameRenderTargetPassTextureList(const ResourceContext& a_resourceContext, const Renderer& a_renderer, const RenderGraphFrameResource& a_renderGraphFrameResource) const;
		void ClearCurrentFrameDepthStencilPassTextureList(const ResourceContext& a_resourceContext, const Renderer& a_renderer, const RenderGraphFrameResource& a_renderGraphFrameResource) const;

		bool ClearRenderTargetPassTexture(const ResourceContext& a_resourceContext, const Renderer& a_renderer, const RenderTargetPassTexture& a_renderTargetPassTexture) const;
		bool ClearDepthStencilPassTexture(const ResourceContext& a_resourceContext, const Renderer& a_renderer, const DepthStencilPassTexture& a_depthStencilPassTexture) const;
	};
}