#pragma once

namespace FWK::Physics
{
	class PhysicsCharacterVirtualAffectedByGravity final : public PhysicsCharacterVirtualBase
	{
	public:

		 PhysicsCharacterVirtualAffectedByGravity()          = default;
		~PhysicsCharacterVirtualAffectedByGravity() override = default;

	protected:

		JPH::Vec3 CalculateLinearVelocity(const JPH::Vec3&                                 a_physicsGravity, 
										  const Struct::PhysicsCharacterVirtualUpdateData& a_updateData, 
			                              const float                                      a_deltaTime,
			                                    JPH::CharacterVirtual&                     a_characterVirtual) override;

		JPH::Vec3 FetchVALUpdateGravity(const JPH::Vec3& a_physicsGravity) const override;

		void ApplyExtendedUpdateSettings(const JPH::CharacterVirtual& a_characterVirtual, JPH::CharacterVirtual::ExtendedUpdateSettings& a_extendedUpdateSettings) const override;

		FWK_DEFINE_TYPE_INFO(PhysicsCharacterVirtualAffectedByGravity, PhysicsCharacterVirtualBase)
	};
}