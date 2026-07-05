#pragma once

namespace FWK::Physics
{
	class PhysicsStaticBodyBase : public PhysicsBodyBase
	{
	public:

		 PhysicsStaticBodyBase()          = default;
		~PhysicsStaticBodyBase() override = default;

	protected:

		bool CreateAndAddStaticBody(const JPH::RefConst<JPH::Shape>& a_shape);

		bool ApplyStaticBodyShape(const JPH::RefConst<JPH::Shape>& a_shape);
	};
}