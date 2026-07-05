#pragma once

namespace FWK::Physics
{
	class PhysicsCharacterVirtualBase
	{
	private:

		friend class Scene;

	public:

		         PhysicsCharacterVirtualBase();
		virtual ~PhysicsCharacterVirtualBase();

		PhysicsCharacterVirtualBase(const PhysicsCharacterVirtualBase&)  = delete;
		PhysicsCharacterVirtualBase(      PhysicsCharacterVirtualBase&&) = delete;

		PhysicsCharacterVirtualBase& operator=(const PhysicsCharacterVirtualBase&)  = delete;
		PhysicsCharacterVirtualBase& operator=(      PhysicsCharacterVirtualBase&&) = delete;

		bool CreeateCharacterVirtual();

		bool RecreateCharacterVirtual();

		void ReleaseCharacterVirtual();

		void Update(const Struct::PhysicsCharacterVirtualUpdateData& a_updateData, const float a_deltaTime);

		void DrawDebug(const JPH::ColorArg a_color) const;

		TypeAlias::Math::Vector3 FetchVALWorldPosition () const;
		TypeAlias::Math::Vector3 FetchVALLinearVelocity() const;

		bool FetchVALIsOnGround() const;

	protected:

		virtual JPH::Vec3 CalculateLinearVelocity(const JPH::Vec3&                                 a_physicsGravity, 
												  const Struct::PhysicsCharacterVirtualUpdateData& a_updateData, 
			                                      const float                                      a_deltaTime, 
			                                            JPH::CharacterVirtual&                     a_characterVirtual);
		
		virtual JPH::Vec3 FetchVALUpdateGravity(const JPH::Vec3 a_physicsGravity) const = 0;

		virtual void ApplyExtendedUpdateSettings(const JPH::CharacterVirtual& a_characterVirtual, JPH::CharacterVirtual::ExtendedUpdateSettings& a_extendedUpdataSettings);

	private:

		JPH::RefConst<JPH::Shape> CreateShape() const;

		bool ApplyShapeChange();

		static constexpr JPH::uint64 k_defaultUserData = 0ULL;

		JPH::Ref<JPH::CharacterVirtual>	m_characterVirtual;

		TypeAlias::Math::Vector3 m_createWorldPosition;

		JPH::CharacterVirtual::ExtendedUpdateSettings m_extendedUpdateSettings;

		float m_capsuleHalfHeightOfCylinder;
		float m_capsuleRadius;
		float m_maxSlopeAngleRadians;
		
		bool m_isEnhancedInternalEdgeRemovealDisabled;
	};
}