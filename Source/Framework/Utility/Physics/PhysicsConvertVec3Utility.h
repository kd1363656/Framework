#pragma once

namespace FWK::Utility
{
	inline JPH::Vec3 DirectXMathVector3ToJoltVec3(const TypeAlias::Math::Vector3& a_vector)
	{
		return JPH::Vec3{ a_vector.x, a_vector.y, a_vector.z };
	}
	inline JPH::RVec3 DirectXMathVector3ToJoltRVec3(const TypeAlias::Math::Vector3& a_vector)
	{
		return JPH::RVec3{ a_vector.x, a_vector.y, a_vector.z };
	}

	inline TypeAlias::Math::Vector3 JoltVec3ToDirectXMathVector3(const JPH::Vec3& a_vector)
	{
		return TypeAlias::Math::Vector3{ a_vector.GetX(), a_vector.GetY(), a_vector.GetZ() };
	}
	inline TypeAlias::Math::Vector3 JoltRVec3ToDirectXMathVector3(const JPH::RVec3& a_vector)
	{
		return TypeAlias::Math::Vector3{ a_vector.GetX(), a_vector.GetY(), a_vector.GetZ() };
	}
}