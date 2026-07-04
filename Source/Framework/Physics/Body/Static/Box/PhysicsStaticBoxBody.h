#pragma once

namespace FWK::Physics
{
	class PhysicsStaticBoxBody final : public PhysicsStaticBodyBase
	{
	public:

		explicit PhysicsStaticBoxBody(const JPH::BodyID a_bodyID);

		 PhysicsStaticBoxBody();
		~PhysicsStaticBoxBody() override;

	private:

		static constexpr float k_minValidHalfExtentLength = 0.0F;
	};
}