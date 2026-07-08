#pragma once

namespace FWK::Utility
{
	// クオータニオンからオイラー角(度)に戻す
	inline TypeAlias::Math::Vector3 QuaternionToEuler(const TypeAlias::Math::Quaternion& a_quaternion)
	{
		const auto& l_radian = a_quaternion.ToEuler();

		return
		{
			DirectX::XMConvertToDegrees(l_radian.x) ,
			DirectX::XMConvertToDegrees(l_radian.y) ,
			DirectX::XMConvertToDegrees(l_radian.z)
		};
	}
	
	// オイラー角(度)からクオータニオンに変換
	inline TypeAlias::Math::Quaternion EulerToQuaternion(const TypeAlias::Math::Vector3& a_euler)
	{
		return
		{
			TypeAlias::Math::Quaternion::CreateFromYawPitchRoll
			(
				DirectX::XMConvertToRadians(a_euler.y) ,
				DirectX::XMConvertToRadians(a_euler.x) ,
				DirectX::XMConvertToRadians(a_euler.z)
			)
		};
	}
}