#pragma once

namespace FWK::Struct
{
	struct PhysicsRay final
	{
		TypeAlias::Math::Vector3 m_startPosition      = {};
		TypeAlias::Math::Vector3 m_directionAndLength = {};
	};

	struct PhysicsRayCastHitResult final
	{
		static constexpr float k_initialHitFraction = 0.0F;
		static constexpr float k_initialHitDistance = 0.0F;

		JPH::BodyID     m_bodyID      = {};
		JPH::SubShapeID m_subShadpeID = {};

		TypeAlias::Math::Vector3 m_hitWorldPosition = TypeAlias::Math::Vector3::Zero;
		TypeAlias::Math::Vector3 m_surfaceNormal    = TypeAlias::Math::Vector3::Zero;

		float m_hitFraction = k_initialHitFraction;
		float m_hitDistance = k_initialHitDistance;

		bool m_isHit = false;
	};
}