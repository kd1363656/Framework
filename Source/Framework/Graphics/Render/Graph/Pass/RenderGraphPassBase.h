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

		virtual void Execute(const ResourceContext& a_resourceContext, Renderer& a_renderer, RenderGraph& a_renderGraph) = 0;

		const auto& GetREFResourceAccessList() const { return m_resourceAccessList; }

		auto GetVALExecutionLayer() const { return m_executionLayer; }

	protected:

		void WriteBackBuffer(const Enum::RenderGraphResourceUsage a_beforeUsage, const Enum::RenderGraphResourceUsage a_afterUsage = Enum::RenderGraphResourceUsage::None);

		void ReadRenderTarget(const Enum::RenderGraphRenderTargetType a_renderTargetType, const Enum::RenderGraphResourceUsage a_beforeUsage, const Enum::RenderGraphResourceUsage a_afterUsage = Enum::RenderGraphResourceUsage::None);
		void ReadDepthStencil(const Enum::RenderGraphDepthStencilType a_depthStencilType, const Enum::RenderGraphResourceUsage a_beforeUsage, const Enum::RenderGraphResourceUsage a_afterUsage = Enum::RenderGraphResourceUsage::None);
		void ReadShadowMap   (const Enum::RenderGraphShadowMapType    a_shadowMapType,    const Enum::RenderGraphResourceUsage a_beforeUsage, const Enum::RenderGraphResourceUsage a_afterUsage = Enum::RenderGraphResourceUsage::None);

		void WriteRenderTarget(const Enum::RenderGraphRenderTargetType a_renderTargetType, const Enum::RenderGraphResourceUsage a_beforeUsage, const Enum::RenderGraphResourceUsage a_afterUsage = Enum::RenderGraphResourceUsage::None);
		void WriteDepthStencil(const Enum::RenderGraphDepthStencilType a_depthStencilType, const Enum::RenderGraphResourceUsage a_beforeUsage, const Enum::RenderGraphResourceUsage a_afterUsage = Enum::RenderGraphResourceUsage::None);
		void WriteShadowMap   (const Enum::RenderGraphShadowMapType    a_shadowMapType,    const Enum::RenderGraphResourceUsage a_beforeUsage, const Enum::RenderGraphResourceUsage a_afterUsage = Enum::RenderGraphResourceUsage::None);

		std::weak_ptr<RootSignature> SetupGraphicsRenderPipeline(Renderer& a_renderer, const Enum::PipelineStateType a_pipelineStateType) const;
		std::weak_ptr<RootSignature> SetupComputeRenderPipeline (Renderer& a_renderer, const Enum::PipelineStateType a_pipelineStateType) const;
		
		void SetupExecutionLayer(const Enum::RenderGraphPassExecutionLayer a_executionLayer);

	private:

		void AddResourceAccess(const bool						       a_isBackBuffer,
							   const Enum::RenderGraphRenderTargetType a_renderTargetType,
							   const Enum::RenderGraphDepthStencilType a_depthStencilType,
							   const Enum::RenderGraphShadowMapType    a_shadowMapType,
							   const Enum::RenderGraphAccessType       a_accessType,
							   const Enum::RenderGraphResourceUsage    a_beforeUsage,
							   const Enum::RenderGraphResourceUsage    a_afterUsage);

		static constexpr bool k_isBackBuffer    = true;
		static constexpr bool k_isNotBackBuffer = false;

		std::vector<Struct::RenderGraphResourceAccess> m_resourceAccessList = {};

		Enum::RenderGraphPassExecutionLayer m_executionLayer = Enum::RenderGraphPassExecutionLayer::Invalid;

		FWK_DEFINE_TYPE_INFO_ROOT(RenderGraphPassBase)
	};
}