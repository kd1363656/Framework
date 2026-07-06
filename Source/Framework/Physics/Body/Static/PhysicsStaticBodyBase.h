#pragma once

namespace FWK::Physics
{
	class PhysicsStaticBodyBase : public PhysicsBodyBase
	{
	public:

		 PhysicsStaticBodyBase()          = default;
		~PhysicsStaticBodyBase() override = default;

	protected:

		bool CreateAndAddStaticBody(const JPH::RefConst<JPH::Shape>&   a_shape, 
			                        const TypeAlias::Math::Vector3&    a_worldPosition,
								    const TypeAlias::Math::Quaternion& a_worldRotation, 
			                        const bool                         a_isPushBackEnabled);

		bool ApplyStaticBodyWorldTransform(const TypeAlias::Math::Vector3&  a_worldPosition, const TypeAlias::Math::Quaternion& a_worldRotation);
		bool ApplyStaticBodyShape         (const JPH::RefConst<JPH::Shape>& a_shape);

		FWK_DEFINE_TYPE_INFO(PhysicsStaticBodyBase, PhysicsBodyBase)
	};
}