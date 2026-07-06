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

		bool CreateBody(const TypeAlias::Math::Quaternion& a_worldRotation, const TypeAlias::Math::Vector3& a_worldPosition, const bool a_isPushBackEnabled) override;

		bool ApplyWorldTransform(const TypeAlias::Math::Quaternion& a_worldRotation, const TypeAlias::Math::Vector3& a_worldPosition) override;
		
	private:

		JPH::RefConst<JPH::Shape> CreateShape() const;

		bool ApplyShapeChange();

		static constexpr float k_defaultRadius = 0.5F;

		float m_radius = k_defaultRadius;

		FWK_DEFINE_TYPE_INFO(PhysicsStaticSphereBody, PhysicsStaticBodyBase)
	};
}