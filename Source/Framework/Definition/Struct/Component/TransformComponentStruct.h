#pragma once

namespace FWK::Struct
{
	struct Transform final
	{
		TypeAlias::Math::Vector3    m_scale    = TypeAlias::Math::Vector3::One;
		TypeAlias::Math::Quaternion m_rotation = TypeAlias::Math::Quaternion::Identity;
		TypeAlias::Math::Vector3    m_position = TypeAlias::Math::Vector3::Zero;
		TypeAlias::Math::Matrix     m_matrix   = TypeAlias::Math::Matrix::Identity;
	};
}