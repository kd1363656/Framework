#pragma once

namespace FWK::Struct
{
	struct SpriteRECT final
	{
		static constexpr UINT k_defaultSpriteRECTX      = 0U;
		static constexpr UINT k_defaultSpriteRECTY      = 0U;
		static constexpr UINT k_defaultSpriteRECTWidth  = 0U;
		static constexpr UINT k_defaultSpriteRECTHeight = 0U;

		std::uint32_t m_x      = k_defaultSpriteRECTX;
		std::uint32_t m_y      = k_defaultSpriteRECTY;
		std::uint32_t m_width  = k_defaultSpriteRECTWidth;
		std::uint32_t m_height = k_defaultSpriteRECTHeight;
	};
}