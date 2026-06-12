#pragma once

namespace FWK::Struct
{
	// スプライト用
	struct SpriteScreenPerObjectDrawRequestData final
	{
		std::weak_ptr<Graphics::TextureRecord> m_textureRecord = {};

		TypeAlias::Math::Color m_color = {};

		TypeAlias::Math::Vector2 m_position = TypeAlias::Math::Vector2::Zero;
		TypeAlias::Math::Vector2 m_scale    = TypeAlias::Math::Vector2::One;
		TypeAlias::Math::Vector2 m_pivot    = Constant::k_defaultSpritePivot;

		SpriteRECT m_sourceRECT = {};
	};
}