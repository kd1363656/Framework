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

		virtual bool CreateBody(const TypeAlias::Math::Quaternion&,            const TypeAlias::Math::Vector3&, const bool)               { return false; }
		virtual bool CreateBody(const Graphics::StaticModelRecord::ModelData&, const bool,                      TypeAlias::Math::Matrix&) { return false; }

		virtual void Deserialize(const nlohmann::json&) { /*必要に応じてオーバーライドしてください*/ };

		virtual nlohmann::json Serialize() const { return nlohmann::json{}; }

		virtual bool ApplyWorldTransform(const TypeAlias::Math::Quaternion&, const TypeAlias::Math::Vector3&) { return false; }
		virtual bool ApplyWorldTransform(      TypeAlias::Math::Matrix&)                                      { return false; }
		
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