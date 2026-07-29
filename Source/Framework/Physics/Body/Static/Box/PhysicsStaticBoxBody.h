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

		bool CreateBody(const TypeAlias::Math::Quaternion& a_worldRotation, const TypeAlias::Math::Vector3& a_worldPosition, const bool a_isPushBackEnabled);

		bool ApplyWorldTransform(const TypeAlias::Math::Quaternion& a_worldRotation, const TypeAlias::Math::Vector3& a_worldPosition) override;

		void SetHalfExtent(const TypeAlias::Math::Vector3& a_set) { m_halfExtent = a_set; }

		const auto& GetRERHalfExtent() const { return m_halfExtent; }

	private:

		JPH::RefConst<JPH::Shape> CreateShape() const;

		bool ApplyShapeChange();

		TypeAlias::Math::Vector3 m_halfExtent = { Constant::k_defaultPhysicsStaticBoxBodyHalfExtentLength, Constant::k_defaultPhysicsStaticBoxBodyHalfExtentLength, Constant::k_defaultPhysicsStaticBoxBodyHalfExtentLength };

		FWK_DEFINE_TYPE_INFO(PhysicsStaticBoxBody, PhysicsStaticBodyBase)
	};
}