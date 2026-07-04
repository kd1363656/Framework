#pragma once

namespace FWK::Physics
{
	class PhysicsStaticBodyBase : public PhysicsBodyBase
	{
	public:

		explicit PhysicsStaticBodyBase(const JPH::BodyID& a_bodyID);

	public:

		 PhysicsStaticBodyBase();
		~PhysicsStaticBodyBase() override;

	protected:

		static JPH::BodyID CreateAndAddStaticBody(const JPH::RefConst<JPH::Shape>& a_shape, const TypeAlias::Math::Vector3& a_worldPosition);
	};
}