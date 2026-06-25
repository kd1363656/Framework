#pragma once

namespace FWK::Physics
{
	class PhysicsBodyCreator final
	{
	public:

		 PhysicsBodyCreator() = default;
		~PhysicsBodyCreator() = default;

		Struct::PhysicsBodyHandle CreateStaticBoxBody(const PhysicsLayerSetting&      a_physicsLayerSetting,
													  const TypeAlias::Math::Vector3& a_worldPosition,
													  const TypeAlias::Math::Vector3& a_halfExtent,
															JPH::PhysicsSystem&		  a_physicsSystem) const;
		
		Struct::PhysicsBodyHandle CreateDynamicSphereBody(const PhysicsLayerSetting&      a_physicsLayerSetting,
														  const TypeAlias::Math::Vector3& a_worldPosition,
														  const float					  a_radius,
														  	    JPH::PhysicsSystem&	      a_physicsSystem) const;

	private:

		JPH::Vec3 ConvertToJoltVector3(const TypeAlias::Math::Vector3& a_vector) const;

		static constexpr float k_minValidBoxHalfExtentLength = 0.0F;
		static constexpr float k_minValidSphereRadius		 = 0.0F;
	};
}