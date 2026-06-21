#pragma once

namespace FWK::Graphics
{
	class Renderer;
	class RenderGraph;
}

namespace FWK::Graphics
{
	class RenderGraphPassBase
	{
	public:

				 RenderGraphPassBase() = default;
		virtual ~RenderGraphPassBase() = default;

		RenderGraphPassBase(const RenderGraphPassBase&)  = delete;
		RenderGraphPassBase(	  RenderGraphPassBase&&) = delete;

		RenderGraphPassBase& operator=(const RenderGraphPassBase&)  = delete;
		RenderGraphPassBase& operator=(	     RenderGraphPassBase&&) = delete;

		virtual void Execute(Renderer& a_renderer, RenderGraph& a_renderGraph) = 0;

		const auto& GetREFResourceAccessList() const { return m_resourceAccessList; }

	protected:

		void WriteBackBuffer(const Enum::RenderGraphResourceUsage a_beforeUsage, const Enum::RenderGraphResourceUsage a_afterUsage = Enum::RenderGraphResourceUsage::None);

		void ReadRenderTarget(const Enum::RenderGraphRenderTargetType a_renderTargetType, const Enum::RenderGraphResourceUsage a_beforeUsage, const Enum::RenderGraphResourceUsage a_afterUsage = Enum::RenderGraphResourceUsage::None);
		void ReadDepthStencil(const Enum::RenderGraphDepthStencilType a_depthStencilType, const Enum::RenderGraphResourceUsage a_beforeUsage, const Enum::RenderGraphResourceUsage a_afterUsage = Enum::RenderGraphResourceUsage::None);

		void WriteRenderTarget(const Enum::RenderGraphRenderTargetType a_renderTargetType, const Enum::RenderGraphResourceUsage a_beforeUsage, const Enum::RenderGraphResourceUsage a_afterUsage = Enum::RenderGraphResourceUsage::None);
		void WriteDepthStencil(const Enum::RenderGraphDepthStencilType a_depthStencilType, const Enum::RenderGraphResourceUsage a_beforeUsage, const Enum::RenderGraphResourceUsage a_afterUsage = Enum::RenderGraphResourceUsage::None);

		std::weak_ptr<RootSignature> SetupRenderPipeline(const Renderer& a_renderer, const Enum::PipelineStateType a_pipelineStateType) const;

	private:

		void AddResourceAccess(const bool						       a_isBackBuffer,
							   const Enum::RenderGraphRenderTargetType a_renderTargetType,
							   const Enum::RenderGraphDepthStencilType a_depthStencilType,
							   const Enum::RenderGraphAccessType       a_accessType,
							   const Enum::RenderGraphResourceUsage    a_beforeUsage,
							   const Enum::RenderGraphResourceUsage    a_afterUsage);

		std::vector<Struct::RenderGraphResourceAccess> m_resourceAccessList = {};

		FWK_DEFINE_TYPE_INFO_ROOT(RenderGraphPassBase)
	};
}