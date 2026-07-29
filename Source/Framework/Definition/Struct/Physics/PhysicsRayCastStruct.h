#pragma once

namespace FWK::Struct
{
	struct PhysicsRay final
	{
		static constexpr float k_initialLength = 1.0F;

		TypeAlias::Math::Vector3 m_worldStartPosition = {};
		TypeAlias::Math::Vector3 m_worldDirection     = {};

		float m_length = k_initialLength;
	};

	struct PhysicsRayCastHitResult final
	{
		static constexpr float k_initialHitFraction = 0.0F;
		static constexpr float k_initialHitDistance = 0.0F;

		JPH::BodyID     m_bodyID     = {};
		JPH::SubShapeID m_subShapeID = {};

		TypeAlias::Math::Vector3 m_worldHitPosition   = TypeAlias::Math::Vector3::Zero;
		TypeAlias::Math::Vector3 m_worldSurfaceNormal = TypeAlias::Math::Vector3::Zero;

		float m_hitFraction = k_initialHitFraction;
		float m_hitDistance = k_initialHitDistance;

		bool m_isHit = false;
	};
}