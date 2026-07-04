#pragma once

namespace FWK::Physics
{
	class PhysicsBodyRegistry final
	{
	public:

		 PhysicsBodyRegistry();
		~PhysicsBodyRegistry();

		PhysicsBodyRegistry(const PhysicsBodyRegistry&)  = delete;
		PhysicsBodyRegistry(	  PhysicsBodyRegistry&&) = delete;

		PhysicsBodyRegistry& operator=(const PhysicsBodyRegistry&)  = delete;
		PhysicsBodyRegistry& operator=(	     PhysicsBodyRegistry&&) = delete;

		bool Setup(const std::shared_ptr<JPH::PhysicsSystem>& a_physicsSystem, const std::shared_ptr<PhysicsLayerSetting>& a_physicsLayerSetting);

		void ReleaseBody(Struct::PhysicsBodyHandle& a_bodyHandle);

		Struct::PhysicsBodyHandle CreateStaticSphereBody (const TypeAlias::Math::Vector3& a_worldPosition, const float					   a_radius);
		Struct::PhysicsBodyHandle CreateStaticBoxBody    (const TypeAlias::Math::Vector3& a_worldPosition, const TypeAlias::Math::Vector3& a_halfExtent);
		Struct::PhysicsBodyHandle CreateStaticCapsuleBody(const TypeAlias::Math::Vector3& a_worldPosition, const float					   a_halfHeightOfCylinder, const float a_radius);
		
		TypeAlias::Math::Vector3 FetchVALBodyWorldPosition(const Struct::PhysicsBodyHandle& a_bodyHandle) const;

	private:

		void RegisterActiveBodyID(const Struct::PhysicsBodyHandle& a_bodyHandle);

		void UnregisterActiveBodyID(const JPH::BodyID& a_bodyID);

		void ReleaseAllBodies();

		std::uint32_t FetchVALBodyIDKey(const JPH::BodyID& a_bodyID) const;

		static constexpr std::size_t k_lastElementIndexOffset = 1ULL;

		std::unordered_map<std::uint32_t, std::size_t> m_activeBodyIDIndexMap;

		std::vector<JPH::BodyID> m_activeBodyIDList;

		std::weak_ptr<JPH::PhysicsSystem>  m_physicsSystem;
		std::weak_ptr<PhysicsLayerSetting> m_physicsLayerSetting;

		PhysicsBodyCreator m_bodyCreator;

		bool m_isSetup;
	};
}