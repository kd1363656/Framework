#pragma once

namespace FWK::Constant
{
	inline constexpr TypeAlias::Math::Color k_colorLess = { 1.0F, 1.0F, 1.0F, 1.0F };

	inline constexpr TypeAlias::Math::Vector2 k_defaultSpritePivot = { 0.5F, 0.5F };

	inline constexpr UINT k_defaultSpriteRECTX      = 0U;
	inline constexpr UINT k_defaultSpriteRECTY      = 0U;
	inline constexpr UINT k_defaultSpriteRECTWidth  = 0U;
	inline constexpr UINT k_defaultSpriteRECTHeight = 0U;

	inline constexpr TypeAlias::DescriptorIndex k_invalidDescriptorIndex = std::numeric_limits<TypeAlias::DescriptorIndex>::max();
}