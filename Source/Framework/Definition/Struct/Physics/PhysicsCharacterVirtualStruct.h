#pragma once

namespace FWK::Struct
{
	struct PhysicsCharacterVirtualUpdateData final
	{
		TypeAlias::Math::Vector3 m_desiredVelocity = {};

		float m_jumpSpeed = Constant::k_defaultCharacterVirtualJumpSpeed;

		bool m_isJumpRequested = false;
	};
}