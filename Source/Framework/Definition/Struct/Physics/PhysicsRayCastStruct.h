#pragma once

namespace FWK::Struct
{
    struct PhysicsRay final
    {
        TypeAlias::Math::Vector3 m_worldStartPosition = {};
        TypeAlias::Math::Vector3 m_worldDirection     = {};

        float m_length = Constant::k_initialPhysicsRayLength;
    };

    struct PhysicsRayCastHitResult final
    {
        JPH::BodyID     m_bodyID     = {};
        JPH::SubShapeID m_subShapeID = {};

        TypeAlias::Math::Vector3 m_worldHitPosition   = TypeAlias::Math::Vector3::Zero;
        TypeAlias::Math::Vector3 m_worldSurfaceNormal = TypeAlias::Math::Vector3::Zero;

        float m_hitFraction = Constant::k_initialPhysicsRayCastHitFraction;
        float m_hitDistance = Constant::k_initialPhysicsRayCastHitDistance;

        bool m_isHit = false;
    };
}