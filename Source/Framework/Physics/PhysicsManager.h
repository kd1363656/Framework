#pragma once

namespace FWK::Physics 
{
	class PhysicsManager final : public Utility::SingletonBase<PhysicsManager>
	{
	private:

		friend class SingletonBase<PhysicsManager>;

		 PhysicsManager();
		~PhysicsManager() override;

	public:

		void INIT();

		void OptimizeBroadPhase();

		void CollectPhysicsDebugDrawCommands();

		void				 ReleaseBody		    (	   Struct::PhysicsBodyHandle& a_bodyHandle);
		TypeAlias::StorageID ReleaseCharacterVirtual(const TypeAlias::StorageID	      a_characterVirtualStorageID);

		void TogglePhysicsDebugDraw();

		Struct::PhysicsBodyHandle CreateStaticSphereBody (const TypeAlias::Math::Vector3& a_worldPosition, const float					   a_radius);
		Struct::PhysicsBodyHandle CreateStaticBoxBody    (const TypeAlias::Math::Vector3& a_worldPosition, const TypeAlias::Math::Vector3& a_halfExtent);
		Struct::PhysicsBodyHandle CreateStaticCapsuleBody(const TypeAlias::Math::Vector3& a_worldPosition, const float					   a_halfHeightOfCylinder, const float a_radius);
		
		TypeAlias::StorageID CreateCharacterVirtual(const Struct::PhysicsCharacterVirtualCreateSetting& a_createSetting);

		TypeAlias::Math::Vector3 FetchVALBodyWorldPosition (const Struct::PhysicsBodyHandle& a_bodyHandle) const;
		
		TypeAlias::Math::Vector3 FetchVALCharacterVirtualWorldPosition (const TypeAlias::StorageID a_characterVirtualStorageID) const;
		TypeAlias::Math::Vector3 FetchVALCharacterVirtualLinearVelocity(const TypeAlias::StorageID a_characterVirtualStorageID) const;
		bool					 FetchVALIsCharacterVirtualOnGrounds   (const TypeAlias::StorageID a_characterVirtualStorageID) const;

		std::weak_ptr<PhysicsDebugRenderer> GetVALDebugRenderer() const { return m_debugRenderer; }

		bool GetVALIsDisableDebugDraw() const { return m_isDisableDebugDraw; }

	private:

		bool SetupJoltCore               ();
		bool SetupSystem                 ();
		
#if defined(_DEBUG)
		static void TraceJoltMessage(const char* a_format, ...);

		void SetupJoltDebugCallback() const;
		
		static constexpr size_t k_joltTraceBufferSize = 2048U;

#ifdef JPH_ENABLE_ASSERTS

		static bool HandleJoltAssertFailed(const char*     a_expression,
										   const char*     a_message,
										   const char*     a_file,
										   const JPH::uint a_line);
#endif

#endif
		void UpdateCharacterVirtual(const Struct::PhysicsCharacterVirtualUpdateData& a_updateData, const float a_deltaTime, CharacterVirtualRecord& a_characterVirtualRecord);

		void Release();

		static constexpr std::size_t k_emptyCharacterVirtualCount = 0ULL;

		static constexpr JPH::uint k_maxBodyCount = 1024U;

		static constexpr JPH::uint k_bodyMutexCount = 0U;

		static constexpr JPH::uint k_maxBodyPairCount = 1024U;

		static constexpr JPH::uint k_maxContactConstraintCount = 1024U;

		static constexpr uint32_t k_tempAllocatorSizeMB = 10U;
		static constexpr uint32_t k_kiloBytePerMB       = 1024U;
		static constexpr uint32_t k_bytePerKB           = 1024U;

		std::unique_ptr<JPH::Factory> m_factory;

		std::shared_ptr<JPH::TempAllocatorImpl> m_tempAllocator;

		std::shared_ptr<PhysicsLayerSetting> m_physicsLayerSetting;
		
		std::shared_ptr<PhysicsDebugRenderer> m_debugRenderer;

		std::shared_ptr<JPH::PhysicsSystem> m_physicsSystem;

		PhysicsBodyRegistry			    m_bodyRegistry;
		PhysicsCharacterVirtualRegistry m_characterVirtualRegistry;

		bool m_isInitialized;

		bool m_isJoltTypeRegistered;

		bool m_isDisableDebugDraw;
	};
}