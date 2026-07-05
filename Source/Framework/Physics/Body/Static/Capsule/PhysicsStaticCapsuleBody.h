#pragma once

namespace FWK::Physics
{
	class PhysicsStaticCapsuleBody final : public PhysicsStaticBodyBase
	{
	private:

		// TODO
		friend class Scene;


	public:

		 PhysicsStaticCapsuleBody()          = default;
		~PhysicsStaticCapsuleBody() override = default;

		bool CreateBody() override;

	private:

		JPH::RefConst<JPH::Shape> CreateShape() const override;

		bool ApplyShapeChange() override;

		static constexpr float k_minHalfHeightOfCylinder     = 0.0F;
		static constexpr float k_minRadius                   = 0.0F;
		static constexpr float k_defaultHalfHeightOfCylinder = 0.5F;
		static constexpr float k_defaultRadius               = 0.5F;

		float m_halfHeightOfCylinder = k_defaultHalfHeightOfCylinder;
		float m_radius               = k_defaultRadius;
	};
}