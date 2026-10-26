#pragma once

namespace FWK::Physics
{
    class PhysicsStaticCapsuleBody final : public PhysicsStaticBodyBase
    {
    public:

         PhysicsStaticCapsuleBody()          = default;
        ~PhysicsStaticCapsuleBody() override = default;

        bool CreateBody(const float                        a_halfHeightOfCylinder,
                        const float                        a_radius,
                        const TypeAlias::Math::Quaternion& a_worldRotation,
                        const TypeAlias::Math::Vector3&    a_worldPosition,
                        const bool                         a_isPushBackEnabled);

        bool ApplyShapeChange(const float a_halfHeightOfCylinder, const float a_radius);

    private:

        JPH::RefConst<JPH::Shape> CreateShape(const float a_halfHeightOfCylinder, const float a_radius) const;

        FWK_DEFINE_TYPE_INFO(PhysicsStaticCapsuleBody, PhysicsStaticBodyBase)
    };
}