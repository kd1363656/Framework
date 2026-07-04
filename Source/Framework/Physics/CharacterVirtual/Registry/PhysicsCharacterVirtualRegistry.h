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

			Enum::PhysicsCharacterVirtualType m_characterVirtualType = Enum::PhysicsCharacterVirtualType::AffectedByGravity;

			bool m_isReleaseReserved = false;
		};

	public:

		 PhysicsCharacterVirtualRegistry();
		~PhysicsCharacterVirtualRegistry();

		PhysicsCharacterVirtualRegistry(const PhysicsCharacterVirtualRegistry&)  = delete;
		PhysicsCharacterVirtualRegistry(	  PhysicsCharacterVirtualRegistry&&) = delete;

		PhysicsCharacterVirtualRegistry& operator=(const PhysicsCharacterVirtualRegistry&)  = delete;
		PhysicsCharacterVirtualRegistry& operator=(	     PhysicsCharacterVirtualRegistry&&) = delete;

		bool Setup		(const std::shared_ptr<JPH::PhysicsSystem>& a_physicsSystem, const std::shared_ptr<PhysicsLayerSetting>& a_physicsLayerSetting, const std::shared_ptr<JPH::TempAllocatorImpl>& a_tempAllocator);
		void Deserialize(const nlohmann::json&                      a_rootJson);

		void UpdateCharacterVirtual(const TypeAlias::StorageID a_characterVirtualStorageID, const Struct::PhysicsCharacterVirtualUpdateData& a_updateData, const float a_deltaTime);

		nlohmann::json Serialize() const;

		TypeAlias::StorageID ReleaseCharacterVirtual(const TypeAlias::StorageID a_characterVirtualStorageID);

		TypeAlias::StorageID CreateCharacterVirtual(const Struct::PhysicsCharacterVirtualCreateSetting& a_createSetting);

		TypeAlias::Math::Vector3 FetchVALCharacterVirtualWorldPosition (const TypeAlias::StorageID a_characterVirtualStorageID) const;
		TypeAlias::Math::Vector3 FetchVALCharacterVirtualLinearVelocity(const TypeAlias::StorageID a_characterVirtualStorageID) const;

		bool FetchVALIsCharacterVirtualOnGround(const TypeAlias::StorageID a_characterVirtualStorageID) const;

		const auto& GetREFStorageIDAllocator() const { return m_storageIDAllocator; }

		auto& GetMutableREFStorageIDAllocator() { return m_storageIDAllocator; }

	private:

		bool SetupStorage();

		void UpdateCharacterVirtualRecord(const Struct::PhysicsCharacterVirtualUpdateData& a_updateData, const float a_deltaTime, CharacterVirtualRecord& a_characterVirtualRecord);

		void ReleaseAllCharacterVirtuals();

		// StorageIDをそのまま配列Indexとして使用する。
		std::vector<CharacterVirtualRecord> m_characterVirtualRecordList;

		std::weak_ptr<JPH::PhysicsSystem>     m_physicsSystem;
		std::weak_ptr<PhysicsLayerSetting>    m_physicsLayerSetting;
		std::weak_ptr<JPH::TempAllocatorImpl> m_tempAllocator;

		Utility::StorageIDAllocator m_storageIDAllocator;

		Converter::PhysicsCharacterVirtualRegistryJsonConverter m_jsonConverter = {};

		bool m_isSetup;
	};
}