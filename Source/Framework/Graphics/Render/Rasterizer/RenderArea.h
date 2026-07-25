#pragma once

namespace FWK::Graphics
{
	class RenderArea final
	{
	public:

		 RenderArea() = default;
		~RenderArea() = default;

		bool Setup(const UINT a_width, const UINT a_height);

		const auto& GetREFViewport   () const { return m_viewport; }
		const auto& GetREFScissorRECT() const { return m_scissorRECT; }

	private:

		bool Setup(const D3D12_VIEWPORT& a_viewport, const D3D12_RECT& a_scissorRECT);

		static constexpr float k_defaultViewportTopLeftX = 0.0F;
		static constexpr float k_defaultViewportTopLeftY = 0.0F;

		static constexpr float k_invalidViewportSize = 0.0F;

		static constexpr LONG k_defaultScissorRECTLeft = 0L;
		static constexpr LONG k_defaultScissorRECTTop  = 0L;

		D3D12_VIEWPORT m_viewport    = {};
		D3D12_RECT     m_scissorRECT = {};
	};
}