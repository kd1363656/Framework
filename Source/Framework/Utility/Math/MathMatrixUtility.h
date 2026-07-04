#pragma once

namespace FWK::Utility
{
	inline float CalculateWorldMaxScale(const TypeAlias::Math::Matrix& a_worldMatrix) 
	{
		// MeshletBoundsは球なので、非均一スケールでも安全になるように最大スケールを使う
		const float l_scaleXSquared = a_worldMatrix._11 * a_worldMatrix._11 + a_worldMatrix._12 * a_worldMatrix._12 + a_worldMatrix._13 * a_worldMatrix._13;
		const float l_scaleYSquared = a_worldMatrix._21 * a_worldMatrix._21 + a_worldMatrix._22 * a_worldMatrix._22 + a_worldMatrix._23 * a_worldMatrix._23;
		const float l_scaleZSquared = a_worldMatrix._31 * a_worldMatrix._31 + a_worldMatrix._32 * a_worldMatrix._32 + a_worldMatrix._33 * a_worldMatrix._33;

		float l_maxScaleSquared = std::max(l_scaleXSquared, l_scaleYSquared);
		
		l_maxScaleSquared = std::max(l_maxScaleSquared, l_scaleZSquared);

		// sqrtは最後に一回だけ行う
		// Math::Vector3::Lengthを3回呼ぶより無駄が少ない
		return std::sqrt(l_maxScaleSquared);
	}
}