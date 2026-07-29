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

		bool CreateBody(const TypeAlias::Math::Quaternion& a_worldRotation, const TypeAlias::Math::Vector3& a_worldPosition, const bool a_isPushBackEnabled);

		bool ApplyWorldTransform(const TypeAlias::Math::Quaternion& a_worldRotation, const TypeAlias::Math::Vector3& a_worldPosition) override;
		
		void SetRadius(const float a_set) { m_radius = a_set; }

		float GetVALRadius() const { return m_radius; }

	private:

		JPH::RefConst<JPH::Shape> CreateShape() const;

		bool ApplyShapeChange();

		float m_radius = Constant::k_defaultPhysicsStaticSphereBodyRadius;

		FWK_DEFINE_TYPE_INFO(PhysicsStaticSphereBody, PhysicsStaticBodyBase)
	};
}