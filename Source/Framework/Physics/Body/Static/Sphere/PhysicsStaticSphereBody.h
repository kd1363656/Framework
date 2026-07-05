#pragma once

namespace FWK::Physics
{
	class PhysicsStaticSphereBody final : public PhysicsStaticBodyBase
	{
	private:

		// TODO
		friend class Scene;

	public:

		 PhysicsStaticSphereBody()          = default;
		~PhysicsStaticSphereBody() override = default;

		bool CreateBody() override;

	private:

		JPH::RefConst<JPH::Shape> CreateShape() const override;

		bool ApplyShapeChange() override;

		static constexpr float k_minRadius     = 0.0F;
		static constexpr float k_defaultRadius = 0.5F;

		float m_radius = k_defaultRadius;

		FWK_DEFINE_TYPE_INFO(PhysicsStaticSphereBody, PhysicsStaticBodyBase)
	};
}