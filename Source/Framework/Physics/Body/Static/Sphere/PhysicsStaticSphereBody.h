#pragma once

namespace FWK::Physics
{
    class PhysicsStaticSphereBody final : public PhysicsStaticBodyBase
    {
    public:

         PhysicsStaticSphereBody()          = default;
        ~PhysicsStaticSphereBody() override = default;

        bool CreateBody(const float                        a_radius,
                        const TypeAlias::Math::Quaternion& a_worldRotation,
                        const TypeAlias::Math::Vector3&    a_worldPosition,
                        const bool                         a_isPushBackEnabled);

        bool ApplyShapeChange(const float a_radius);

    private:

        JPH::RefConst<JPH::Shape> CreateShape(const float a_radius) const;

        FWK_DEFINE_TYPE_INFO(PhysicsStaticSphereBody, PhysicsStaticBodyBase)
    };
}