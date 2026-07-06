#pragma once

namespace FWK
{
	class Scene;
}

namespace FWK::Physics
{
	class PhysicsBodyBase
	{
	public:

				 PhysicsBodyBase();
		virtual ~PhysicsBodyBase();

		
		PhysicsBodyBase(const PhysicsBodyBase&)  = delete;
		PhysicsBodyBase(      PhysicsBodyBase&&) = delete;

		PhysicsBodyBase& operator=(const PhysicsBodyBase&)  = delete;
		PhysicsBodyBase& operator=(      PhysicsBodyBase&&) = delete;

		virtual bool CreateBody(const TypeAlias::Math::Quaternion& a_worldRotation, const TypeAlias::Math::Vector3& a_worldPosition, bool a_isPushBackEnabled) { return false; }
		virtual bool CreateBody(const TypeAlias::Math::Matrix&     a_worldMatrix,   const Struct::StaticModelData&  a_staticModelData)                         { return false; }

		virtual bool ApplyWorldTransform(const TypeAlias::Math::Vector3& a_worldPosition, const TypeAlias::Math::Quaternion& a_worldRotation) { return false; }
		virtual bool ApplyWorldTransform(const TypeAlias::Math::Matrix&  a_worldMatrix)                                                       { return false; }
		
		bool ApplyIsPushBackEnabled(const bool a_isPushBackEnabled) const;

		TypeAlias::Math::Vector3 FetchVALWorldPosition() const;

	protected:

		bool ApplyBodyShape(const JPH::RefConst<JPH::Shape>& a_shape, const JPH::EActivation a_activationMode, const bool a_isUpdateMassProperties) const;

		void SetBodyID(const JPH::BodyID a_set) { m_bodyID = a_set; }

		const auto& GetREFBodyID() const { return m_bodyID; }

	private:

		void ReleaseBody();

		JPH::BodyID m_bodyID;

		FWK_DEFINE_TYPE_INFO_ROOT(PhysicsBodyBase)
	};
}