#pragma once

namespace FWK::Physics
{
	class PhysicsStaticBoxBody final : public PhysicsStaticBodyBase
	{
	private:

		// TODO
		friend class Scene;

	public:

		 PhysicsStaticBoxBody()          = default;
		~PhysicsStaticBoxBody() override = default;

		bool CreateBody() override;

	private:

		JPH::RefConst<JPH::Shape> CreateShape() const override;

		bool ApplyShapeChange() override;

		static constexpr float k_minHalfExtentLength     = 0.0;
		static constexpr float k_defaultHalfExtentLength = 0.5F;

		TypeAlias::Math::Vector3 m_halfExtent = { k_defaultHalfExtentLength, k_defaultHalfExtentLength, k_defaultHalfExtentLength };
	};
}