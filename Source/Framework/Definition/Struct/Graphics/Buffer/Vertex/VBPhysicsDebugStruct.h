#pragma once

namespace FWK::Struct
{
    struct VBPhysicsDebug final
    {
        TypeAlias::Math::Vector3 m_position = TypeAlias::Math::Vector3::Zero;
        TypeAlias::Math::Color   m_color    = Constant::k_whiteColor;
    };
}