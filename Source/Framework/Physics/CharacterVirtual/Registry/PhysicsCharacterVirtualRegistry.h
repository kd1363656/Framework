#pragma once

namespace FWK::Physics
{
	class PhysicsCharacterVirtualRegistry final
	{
	private:

		struct CharacterVirtualRecord final
		{
			JPH::Ref<JPH::CharacterVirtual> m_characterVirtual = nullptr;

			JPH::CharacterVirtual::ExtendedUpdateSettings m_extendedUpdateSettings = {};

			bool m_isReleaseReserved = false;
		};

	public:

		 PhysicsCharacterVirtualRegistry();
		~PhysicsCharacterVirtualRegistry();

		PhysicsCharacterVirtualRegistry(const PhysicsCharacterVirtualRegistry&)  = delete;
		PhysicsCharacterVirtualRegistry(	  PhysicsCharacterVirtualRegistry&&) = delete;

		PhysicsCharacterVirtualRegistry& operator=(const PhysicsCharacterVirtualRegistry&)  = delete;
		PhysicsCharacterVirtualRegistry& operator=(	     PhysicsCharacterVirtualRegistry&&) = delete;

		bool Setup(const std::shared_ptr<JPH::PhysicsSystem>& a_physicsSystem, const std::shared_ptr<PhysicsLayerSetting>& a_physicsLayerSetting, const std::shared_ptr<JPH::TempAllocatorImpl>& a_tempAllocator);

		void UpdateCharacterVirtual(const TypeAlias::StorageID a_characterVirtualStorageID, const Struct::PhysicsCharacterVirtualUpdateData& a_updateData, const float a_deltaTime);

		TypeAlias::StorageID ReleaseCharacterVirtual(const TypeAlias::StorageID a_characterVirtualStorageID);

		TypeAlias::StorageID CreateCharacterVirtual(const Struct::PhysicsCharacterVirtualCreateSetting& a_createSetting);

		TypeAlias::Math::Vector3 FetchVALCharacterVirtualWorldPosition (const TypeAlias::StorageID a_characterVirtualStorageID) const;
		TypeAlias::Math::Vector3 FetchVALCharacterVirtualLinearVelocity(const TypeAlias::StorageID a_characterVirtualStorageID) const;

		bool FetchVALIsCharacterVirtualOnGround(const TypeAlias::StorageID a_characterVirtualStorageID) const;

	private:

		bool SetupStorage();

		void UpdateCharacterVirtualRecord(const Struct::PhysicsCharacterVirtualUpdateData& a_updateData, const float a_deltaTime, CharacterVirtualRecord& a_characterVirtualRecord);

		void ReleaseAllCharacterVirtuals();

		static constexpr float k_minCharacterVirtualDeltaTime				    = 0.0F;
		static constexpr float k_minCharacterVirtualCapsuleHalfHeightOfCylinder = 0.0F;
		static constexpr float k_minCharacterVirtualCapsuleRadius				= 0.0F;
		static constexpr float k_minCharacterVirtualSlopeAngleRadius			= 0.0F;
		static constexpr float k_minCharacterVirtualMaxSlopeAngleRadians		= 0.0F;
		static constexpr float k_maxCharacterVirtualMaxSlopeAngleRadians		= DirectX::XM_PIDIV2;
		static constexpr float k_maxCharacterVirtualJumpSpeed			        = 0.0F;

		// StorageIDをそのまま配列Indexとして使用する。
		std::vector<CharacterVirtualRecord> m_characterVirtualRecordList;

		std::weak_ptr<JPH::PhysicsSystem>     m_physicsSystem;
		std::weak_ptr<PhysicsLayerSetting>    m_physicsLayerSetting;
		std::weak_ptr<JPH::TempAllocatorImpl> m_tempAllocator;

		Utility::StorageIDAllocator m_storageIDAllocator;

		bool m_isSetup;
	};
}