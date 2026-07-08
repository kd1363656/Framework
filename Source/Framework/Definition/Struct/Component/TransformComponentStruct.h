#pragma once

namespace FWK
{
	class MatrixStrategyBase;
}

namespace FWK::Struct
{
	struct Transform final
	{
		TypeAlias::Math::Vector3    m_scale    = TypeAlias::Math::Vector3::One;
		TypeAlias::Math::Quaternion m_rotation = TypeAlias::Math::Quaternion::Identity;
		TypeAlias::Math::Vector3    m_position = TypeAlias::Math::Vector3::Zero;

		std::unique_ptr<MatrixStrategyBase> m_matrixStrategy = nullptr;
	};
}