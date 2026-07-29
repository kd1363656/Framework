#pragma once

namespace FWK::Struct
{
	struct PhysicsRay final
	{
		static constexpr float k_defaultMAXDistance = 0.0F;

		TypeAlias::Math::Vector3 m_origin = TypeAlias::Math::Vector3::Zero;
		TypeAlias::Math::Vector3 m_direction = TypeAlias::Math::Vector3::Zero;

		float m_maxDistance = k_defaultMAXDistance;
	};
}