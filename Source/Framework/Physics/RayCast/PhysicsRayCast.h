#pragma once

namespace FWK::Physics
{
	class PhysicsRayCast final
	{
	public:

		 PhysicsRayCast() = default;
		~PhysicsRayCast() = default;

		PhysicsRayCast(const PhysicsRayCast&)  = delete;
		PhysicsRayCast(      PhysicsRayCast&&) = delete;

		PhysicsRayCast& operator=(const PhysicsRayCast&)  = delete;
		PhysicsRayCast& operator=(      PhysicsRayCast&&) = delete;

		std::vector<Struct::PhysicsRayCastHitResult> CastAllHitBodies(const Struct::PhysicsRay& a_physicsRay) const;

		Struct::PhysicsRayCastHitResult CastClosestHit(const Struct::PhysicsRay& a_physicsRay) const;

	private:

		bool CreateJoltRay(const Struct::PhysicsRay& a_physicsRay, JPH::RRayCast& a_rRayCast) const;

		Struct::PhysicsRayCastHitResult CreateHitResult(const JPH::RRayCast& a_rRayCast, const JPH::RayCastResult& a_rayCastResult, const float a_rayLength) const;
	};
}