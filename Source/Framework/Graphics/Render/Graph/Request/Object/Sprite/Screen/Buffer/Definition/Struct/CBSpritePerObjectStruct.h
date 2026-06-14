#pragma once

namespace FWK::Struct
{
	// ※ 注意
	// HLSLのConstantBufferは16バイト単位でパッキングされるため、
	// C++側の構造体レイアウトも16バイト境界を意識して定義すること
	struct CBSpritePerObject final
	{
		TypeAlias::Math::Color m_color = Constant::k_colorLess;

		TypeAlias::Math::Vector2 m_position = TypeAlias::Math::Vector2::Zero;
		TypeAlias::Math::Vector2 m_scale    = TypeAlias::Math::Vector2::One;

		TypeAlias::Math::Vector2 m_pivot	    = Constant::k_defaultSpritePivot;
		TypeAlias::Math::Vector2 m_firstPadding = TypeAlias::Math::Vector2::Zero;

		Struct::SpriteRECT m_sourceRECT = {};

		TypeAlias::DescriptorIndex m_baseColorTextureSRVIndex = Constant::k_invalidDescriptorIndex;
		TypeAlias::Math::Vector3   m_secondPadding		      = {};
	};
}