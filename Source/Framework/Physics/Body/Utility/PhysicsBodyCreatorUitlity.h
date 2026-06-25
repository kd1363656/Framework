#pragma once

namespace FWK::Utility
{
	inline JPH::Vec3 ConvertToJoltVector3(const TypeAlias::Math::Vector3& a_vector)
	{
		return JPH::Vec3{ a_vector.x, a_vector.y, a_vector.z };
	}
}