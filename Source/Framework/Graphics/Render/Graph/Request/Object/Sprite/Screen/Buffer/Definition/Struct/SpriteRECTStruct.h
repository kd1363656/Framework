#pragma once

namespace FWK::Struct
{
	struct SpriteRECT final
	{
		std::uint32_t m_x      = Constant::k_defaultSpriteRECTX;
		std::uint32_t m_y      = Constant::k_defaultSpriteRECTY;
		std::uint32_t m_width  = Constant::k_defaultSpriteRECTWidth;
		std::uint32_t m_height = Constant::k_defaultSpriteRECTHeight;
	};
}