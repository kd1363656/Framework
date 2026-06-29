#pragma once

namespace FWK::Constant
{
	inline constexpr float k_spritePositionRateLeft   = 0.0F;
	inline constexpr float k_spritePositionRateRight  = 1.0F;
	inline constexpr float k_spritePositionRateBottom = 0.0F;
	inline constexpr float k_spritePositionRateTop    = 1.0F;

	inline constexpr float k_spriteUVRateLeft   = 0.0F;
	inline constexpr float k_spriteUVRateRight  = 1.0F;
	inline constexpr float k_spriteUVRateTop    = 0.0F;
	inline constexpr float k_spriteUVRateBottom = 1.0F;

	// 四角形の頂点数
	inline constexpr UINT k_spriteVertexCount = 4U;

	// 四角形を構成する三角形の数
	inline constexpr UINT k_spritePrimitiveCount = 2U;

	// SpriteScreen全体のIndex数
	// 2Triangle * 3Index = 6Index
	inline constexpr UINT k_spriteIndexCount = k_spritePrimitiveCount * k_triangleVertexCount;

	inline constexpr UINT k_spriteDrawInstanceCount     = 1U;
	inline constexpr UINT k_spriteStartVertexLocation   = 0U;
	inline constexpr UINT k_spriteStartInstanceLocation = 0U;

	inline constexpr std::uint16_t k_spriteLeftBottomVertexIndex  = 0U;
	inline constexpr std::uint16_t k_spriteLeftTopVertexIndex     = 1U;
	inline constexpr std::uint16_t k_spriteRightBottomVertexIndex = 2U;
	inline constexpr std::uint16_t k_spriteRightTopVertexIndex    = 3U;

	inline constexpr INT k_spriteScreenBaseVertexLocation = 0;
}