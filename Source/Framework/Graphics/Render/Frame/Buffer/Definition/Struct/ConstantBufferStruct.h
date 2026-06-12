#pragma once

namespace FWK::Struct
{
	struct SpriteRECT final
	{
		std::uint32_t m_x      = 0U;
		std::uint32_t m_y      = 0U;
		std::uint32_t m_width  = 0U;
		std::uint32_t m_height = 0U;
	};

	// ※ 注意
	// HLSLのConstantBufferは16バイト単位でパッキングされるため、
	// C++側の構造体レイアウトも16バイト境界を意識して定義すること
	struct CBSpritePerObject final
	{
		TypeAlias::Math::Color m_color = Constant::k_defaultSpriteColor;

		TypeAlias::Math::Vector2 m_position = TypeAlias::Math::Vector2::Zero;
		TypeAlias::Math::Vector2 m_scale    = TypeAlias::Math::Vector2::One;

		TypeAlias::Math::Vector2 m_pivot	    = Constant::k_defaultSpritePivot;
		TypeAlias::Math::Vector2 m_firstPadding = TypeAlias::Math::Vector2::Zero;

		Struct::SpriteRECT m_sourceRECT = {};

		TypeAlias::DescriptorIndex m_baseColorTextureSRVIndex = Constant::k_invalidDescriptorIndex;
		TypeAlias::Math::Vector3   m_secondPadding		      = {};
	};

	struct CBSpritePass final
	{
		TypeAlias::Math::Matrix m_projectionMatrix = TypeAlias::Math::Matrix::Identity;
	};
}