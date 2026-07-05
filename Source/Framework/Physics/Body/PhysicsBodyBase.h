#pragma once

namespace FWK
{
	class Scene;
}

namespace FWK::Physics
{
	class PhysicsBodyBase
	{
	private:

		// TODO
		friend class Scene;

	public:

				 PhysicsBodyBase();
		virtual ~PhysicsBodyBase();

		
		PhysicsBodyBase(const PhysicsBodyBase&)  = delete;
		PhysicsBodyBase(      PhysicsBodyBase&&) = delete;

		PhysicsBodyBase& operator=(const PhysicsBodyBase&)  = delete;
		PhysicsBodyBase& operator=(      PhysicsBodyBase&&) = delete;

		virtual bool CreateBody() = 0;

		TypeAlias::Math::Vector3 FetchVALWorldPosition() const;

		void SetCreateWorldPosition(const TypeAlias::Math::Vector3& a_set) { m_createWorldPosition = a_set; }

	protected:

		bool ApplyBodyShape(const JPH::RefConst<JPH::Shape>& a_shape, const JPH::EActivation a_activationMode, const bool a_isUpdateMassProperties);

		void SetBodyID(const JPH::BodyID a_set) { m_bodyID = a_set; }

		const auto& GetREFBodyID() const { return m_bodyID; }

		const auto& GetREFCreateWorldPosition() const { return m_createWorldPosition; }

	private:

		virtual JPH::RefConst<JPH::Shape> CreateShape() const = 0;

		void ReleaseBody();

		virtual bool ApplyShapeChange() = 0;

		JPH::BodyID m_bodyID;

		TypeAlias::Math::Vector3 m_createWorldPosition;
	};
}