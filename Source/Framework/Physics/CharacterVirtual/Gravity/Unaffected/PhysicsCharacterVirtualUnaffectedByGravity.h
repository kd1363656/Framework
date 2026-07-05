#pragma once

namespace FWK::Physics
{
	class PhysicsCharacterVirtualUnaffectedByGravity final : public PhysicsCharacterVirtualBase
	{
	public:

		 PhysicsCharacterVirtualUnaffectedByGravity()          = default;
		~PhysicsCharacterVirtualUnaffectedByGravity() override = default;

	protected:

		JPH::Vec3 CalculateLinearVelocity(const JPH::Vec3&, 
										  const Struct::PhysicsCharacterVirtualUpdateData& a_updateData,
			                              const float,
			                                    JPH::CharacterVirtual& a_characterVirtual) override;

		JPH::Vec3 FetchVALUpdateGravity(const JPH::Vec3&) const override;

		void ApplyExtendedUpdateSettings(const JPH::CharacterVirtual&, JPH::CharacterVirtual::ExtendedUpdateSettings& a_extendedUpdateSettings) const override;

		FWK_DEFINE_TYPE_INFO(PhysicsCharacterVirtualUnaffectedByGravity, PhysicsCharacterVirtualBase)
	};
}