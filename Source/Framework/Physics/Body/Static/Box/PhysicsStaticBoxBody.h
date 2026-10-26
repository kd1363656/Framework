#pragma once

namespace FWK::Physics
{
    class PhysicsStaticBoxBody final : public PhysicsStaticBodyBase
    {
    public:

         PhysicsStaticBoxBody()          = default;
        ~PhysicsStaticBoxBody() override = default;

        bool CreateBody(const TypeAlias::Math::Vector3&    a_halfExtent,
                        const TypeAlias::Math::Quaternion& a_worldRotation,
                        const TypeAlias::Math::Vector3&    a_worldPosition,
                        const bool                         a_isPushBackEnabled);

        bool ApplyShapeChange(const TypeAlias::Math::Vector3& a_halfExtent);

    private:

        JPH::RefConst<JPH::Shape> CreateShape(const TypeAlias::Math::Vector3& a_halfExtent) const;

        FWK_DEFINE_TYPE_INFO(PhysicsStaticBoxBody, PhysicsStaticBodyBase)
    };
}