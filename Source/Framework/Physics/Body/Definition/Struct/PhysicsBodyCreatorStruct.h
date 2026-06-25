#pragma once

namespace FWK::Struct
{
	struct PhysicsBodyHandle
	{
		JPH::BodyID m_bodyID = {};
		
		bool m_isValid = false;
	};
}