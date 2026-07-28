#pragma once

namespace FWK::Struct
{
	struct StaticModelPerObjectDrawRequestData final
	{
		static constexpr float k_defaultWorldMaxScale = 0.0F;

		std::weak_ptr<Graphics::StaticModelRecord> m_staticModelRecord = {};

		TypeAlias::Math::Matrix m_worldMatrix                 = TypeAlias::Math::Matrix::Identity;
		TypeAlias::Math::Matrix m_worldInverseTransposeMatrix = TypeAlias::Math::Matrix::Identity;

		float m_worldMaxScale = k_defaultWorldMaxScale;
	};

	struct SkeletalAnimationModelPerObjectDrawRequestData final
	{
		std::weak_ptr<Graphics::SkeletalAnimationPlayer> m_skeletalAnimationPlayer = {};

		TypeAlias::Math::Matrix m_worldMatrix                 = TypeAlias::Math::Matrix::Identity;
		TypeAlias::Math::Matrix m_worldInverseTransposeMatrix = TypeAlias::Math::Matrix::Identity;
	};
}