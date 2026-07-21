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

		bool CreateBody(const TypeAlias::Math::Quaternion& a_worldRotation, const TypeAlias::Math::Vector3& a_worldPosition, const bool a_isPushBackEnabled) override;

		void Deserialize(const nlohmann::json& a_rootJson) override;

		nlohmann::json Serialize() const override;

		bool ApplyWorldTransform(const TypeAlias::Math::Quaternion& a_worldRotation, const TypeAlias::Math::Vector3& a_worldPosition) override;
		
		void SetHalfHeightOfCylinder(const float a_set) { m_halfHeightOfCylinder = a_set; }
		void SetRadius			    (const float a_set) { m_radius               = a_set;}

		float GetVALHalfHeightOfCylinder() const { return m_halfHeightOfCylinder; }
		float GetVALRadius              () const { return m_radius; }

	private:

		JPH::RefConst<JPH::Shape> CreateShape() const;

		bool ApplyShapeChange();

		Converter::PhysicsStaticCapsuleBodyJsonConverter m_jsonConverter = {};

		float m_halfHeightOfCylinder = Constant::k_defaultPhysicsStaticCapsuleBodyHalfHeightOfCylinder;
		float m_radius               = Constant::k_defaultPhysicsStaticCapsuleBodyRadius;

		FWK_DEFINE_TYPE_INFO(PhysicsStaticCapsuleBody, PhysicsStaticBodyBase)
	};
}