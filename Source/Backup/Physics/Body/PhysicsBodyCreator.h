#pragma once

namespace FWK::Physics
{
	class PhysicsBodyCreator final
	{
	public:

		 PhysicsBodyCreator() = default;
		~PhysicsBodyCreator() = default;

		// スフィアボディ
		Struct::PhysicsBodyHandle CreateStaticSphereBody(const PhysicsLayerSetting&      a_physicsLayerSetting,
														 const TypeAlias::Math::Vector3& a_worldPosition,
														 const float					 a_radius,
														 	   JPH::PhysicsSystem&	     a_physicsSystem) const;

		// ボックスボディ
		Struct::PhysicsBodyHandle CreateStaticBoxBody(const PhysicsLayerSetting&      a_physicsLayerSetting,
													  const TypeAlias::Math::Vector3& a_worldPosition,
													  const TypeAlias::Math::Vector3& a_halfExtent,
															JPH::PhysicsSystem&		  a_physicsSystem) const;

		// カプセルボディ
		Struct::PhysicsBodyHandle CreateStaticCapsuleBody(const PhysicsLayerSetting&	  a_physicsLayerSetting,
														  const TypeAlias::Math::Vector3& a_worldPosition,
														  const float					  a_halfHeightOfCylinder,
														  const float					  a_radius,
														        JPH::PhysicsSystem&	      a_physicsSystem) const;

	private:

		static constexpr float k_minValidBoxHalfExtentLength = 0.0F;
		static constexpr float k_minValidSphereRadius		 = 0.0F;

		// JoltのCapsuleShapeSettings自体はhalfHeightOfCylinder == 0.0Fを許可する
		// ただし、その場合はカプセルではなくSphereShapeとして作られる
		static constexpr float k_minValidCapsuleHalfHeightOfCylinder = 0.0F;
		static constexpr float k_minValidCapsuleRadius			     = 0.0F;
	};
}