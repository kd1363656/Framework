#pragma once

namespace FWK::Struct
{
    struct SpriteScreenDrawRequestData final
    {
        std::weak_ptr<Graphics::TextureRecord> m_textureRecord = {};

        TypeAlias::Math::Color m_color = Constant::k_whiteColor;

        TypeAlias::Math::Vector2 m_position = TypeAlias::Math::Vector2::Zero;
        TypeAlias::Math::Vector2 m_scale    = TypeAlias::Math::Vector2::One;
        TypeAlias::Math::Vector2 m_pivot    = Constant::k_defaultSpritePivot;

        Struct::SpriteRECT m_sourceRECT = {};
    };
}