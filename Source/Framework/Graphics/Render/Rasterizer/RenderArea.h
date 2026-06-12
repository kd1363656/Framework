#pragma once

namespace FWK::Graphics
{
	class RenderGraph;
}

namespace FWK::Graphics
{
	class RenderArea final
	{
	public:

		 RenderArea() = default;
		~RenderArea() = default;

		bool SetupRenderArea(const SwapChain& a_swapChain);

		void SyncSpritePassDrawRequest(const RenderGraph& a_renderGraph);

		const auto& GetREFViewport   () const { return m_viewport; }
		const auto& GetREFScissorRECT() const { return m_scissorRECT; }

	private:

		static constexpr float k_defaultNearClip = 0.0F;
		static constexpr float k_defaultFarClip  = 1.0F;

		static constexpr LONG k_defaultScissorRECTLeft = 0L;
		static constexpr LONG k_defaultScissorRECTTop  = 0L;

		D3D12_VIEWPORT m_viewport    = {};
		D3D12_RECT     m_scissorRECT = {};

		std::shared_ptr<Struct::CBSpritePass> m_cbSpritePass = nullptr;
	};
}