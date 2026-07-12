#pragma once

namespace FWK::Utility
{
	inline TypeAlias::Math::Vector3 ConvertUFBXVector3ToVector3(const ufbx_vec3& a_fbxVector)
	{
		// ufbx_vec3はdouble系の値を持つため、自作フレームワークのVector3で使うfloatへ変換する
		return TypeAlias::Math::Vector3
		(
			static_cast<float>(a_fbxVector.x),
			static_cast<float>(a_fbxVector.y),
			static_cast<float>(a_fbxVector.z)
		);
	}
	inline TypeAlias::Math::Vector2 ConvertUFBXVector2ToVector2(const ufbx_vec2& a_fbxVector)
	{
		// ufbx_vec2はdouble系の値を持つため、自作フレームワークのVector2で使うfloatへ変換する
		return TypeAlias::Math::Vector2
		(
			static_cast<float>(a_fbxVector.x),
			static_cast<float>(a_fbxVector.y)
		);
	}
}