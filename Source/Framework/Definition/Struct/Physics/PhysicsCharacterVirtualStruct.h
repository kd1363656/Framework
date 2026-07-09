#pragma once

namespace FWK::Struct
{
	struct PhysicsCharacterVirtualUpdateData final
	{
		static constexpr float k_defaultCharacterVirtualJumpSpeed = 5.0F;

		TypeAlias::Math::Vector3 m_desiredVelocity = {};

		float m_jumpSpeed = k_defaultCharacterVirtualJumpSpeed;

		bool m_isJumpRequested = false;
	};
}