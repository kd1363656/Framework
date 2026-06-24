#pragma once

namespace FWK::Physics
{
	class PhysicsLayerSetting final
	{
	public:

		 PhysicsLayerSetting();
		~PhysicsLayerSetting() = default;

		const auto& GetREFBroadPhaseLayerInterface     () const { return m_broadPhaseLayerInterface; }
		const auto& GetREFObjectVSBroadPhaseLayerFilter() const { return m_objectVsBroadPhaseLayerFilter; }
		const auto& GetREFObjectLayerPairFilter        () const { return m_objectLayerPairFilter; }

		JPH::ObjectLayer FetchVALObjectLayer(const Enum::PhysicsObjectLayerType a_objectLayerType) const;
		
	private:

		void SetupBroadPhaseLayerMapping       ();
		void SetupObjectLayerCollisionFilter   ();
		void SetupObjectVSBroadPhaseLayerFilter();

		void EnableObjectLayerCollision(const Enum::PhysicsObjectLayerType a_layerA, const Enum::PhysicsObjectLayerType a_layerB);

		JPH::ObjectLayer     ConvertToJoltObjectLayer    (const Enum::PhysicsObjectLayerType     a_objectLayerType)     const;
		JPH::BroadPhaseLayer ConvertToJoltBroadPhaseLayer(const Enum::PhysicsBroadPhaseLayerType a_broadPhaseLayerType) const;

		JPH::BroadPhaseLayerInterfaceTable m_broadPhaseLayerInterface;

		JPH::ObjectLayerPairFilterTable m_objectLayerPairFilter;

		std::unique_ptr<JPH::ObjectVsBroadPhaseLayerFilterTable> m_objectVsBroadPhaseLayerFilter;
	};
}