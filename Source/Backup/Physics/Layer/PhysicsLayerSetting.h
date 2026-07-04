#pragma once

namespace FWK::Physics
{
	class PhysicsLayerSetting final
	{
	public:

		 PhysicsLayerSetting();
		~PhysicsLayerSetting();

		void INIT();

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

		// ObjectLayerをBroadPhaseLayerに変換する表
		JPH::BroadPhaseLayerInterfaceTable m_broadPhaseLayerInterface;

		// ObjectLayer同士が衝突してよいかを決める表
		JPH::ObjectLayerPairFilterTable m_objectLayerPairFilter;

		// ObjectLayerがどのBroadPhaseLayerを調べる必要があるかを決めるFilter
		std::unique_ptr<JPH::ObjectVsBroadPhaseLayerFilterTable> m_objectVsBroadPhaseLayerFilter;
	};
}