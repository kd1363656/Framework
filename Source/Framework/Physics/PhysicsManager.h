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

		void Update(const float a_deltaTime);

		void OptimizeBroadPhase();

		void ReleaseBody(Struct::PhysicsBodyHandle& a_bodyHandle);

		// スフィアボディ
		Struct::PhysicsBodyHandle CreateStaticSphereBody (const TypeAlias::Math::Vector3& a_worldPosition, const float a_radius);
		Struct::PhysicsBodyHandle CreateDynamicSphereBody(const TypeAlias::Math::Vector3& a_worldPosition, const float a_radius);
		
		// ボックスボディ
		Struct::PhysicsBodyHandle CreateStaticBoxBody (const TypeAlias::Math::Vector3& a_worldPosition, const TypeAlias::Math::Vector3& a_halfExtent);
		Struct::PhysicsBodyHandle CreateDynamicBoxBody(const TypeAlias::Math::Vector3& a_worldPosition, const TypeAlias::Math::Vector3& a_halfExtent);
		
		// カプセルボディ
		Struct::PhysicsBodyHandle CreateStaticCapsuleBody (const TypeAlias::Math::Vector3& a_worldPosition, const float a_halfHeightOfCylinder, const float a_radius);
		Struct::PhysicsBodyHandle CreateDynamicCapsuleBody(const TypeAlias::Math::Vector3& a_worldPosition, const float a_halfHeightOfCylinder, const float a_radius);

		void ApplyBodyLinearVelocity(const Struct::PhysicsBodyHandle& a_bodyHandle, const TypeAlias::Math::Vector3& a_linearVelocity);
		void ApplyBodyGravityEnabled(const Struct::PhysicsBodyHandle& a_bodyHandle, const bool						a_isEnabled);

		TypeAlias::Math::Vector3 FetchVALBodyWorldPosition (const Struct::PhysicsBodyHandle& a_bodyHandle) const;
		TypeAlias::Math::Vector3 FetchVALBodyLinearVelocity(const Struct::PhysicsBodyHandle& a_bodyHandle) const;

	private:

		bool SetupJoltCore     ();
		bool SetupPhysicsSystem();

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

		void RegisterActiveBodyID(const Struct::PhysicsBodyHandle& a_bodyHandle);

		void UnregisterActiveBodyID(const JPH::BodyID& a_bodyID);

		void ReleaseAllBodies();

		void Release();

		std::uint32_t FetchVALBodyIDKey(const JPH::BodyID& a_bodyID) const;

		static constexpr std::size_t k_lastElementIndexOffset = 1ULL;

		static constexpr JPH::uint k_maxBodyCount = 1024U;

		static constexpr JPH::uint k_bodyMutexCount = 0U;

		static constexpr JPH::uint k_maxBodyPairCount = 1024U;

		static constexpr JPH::uint k_maxContactConstraintCount = 1024U;

		static constexpr float k_bodyGravityEnabledFactor  = 1.0F;
		static constexpr float k_bodyGravityDisabledFactor = 0.0F;

		static constexpr uint32_t k_tempAllocatorSizeMB = 10U;
		static constexpr uint32_t k_kiloBytePerMB       = 1024U;
		static constexpr uint32_t k_bytePerKB           = 1024U;

		static constexpr uint32_t k_maxPhysicsJobCount     = 2048U;
		static constexpr uint32_t k_maxPhysicsBarrierCount = 8U;
		static constexpr uint32_t k_mainThreadCount        = 1U;
		static constexpr uint32_t k_minHardwareThreadCount = 1U;

		static constexpr int k_fallbackWorkerThreadCount = 1;
		
		static constexpr int k_collisionStepCount = 1;

		std::unordered_map<std::uint32_t, std::size_t> m_activeBodyIDIndexMap;

		std::vector<JPH::BodyID> m_activeBodyIDList;

		std::unique_ptr<JPH::Factory> m_factory;

		std::unique_ptr<JPH::TempAllocatorImpl> m_tempAllocator;

		std::unique_ptr<JPH::JobSystemThreadPool> m_jobSystem;

		std::unique_ptr<PhysicsLayerSetting> m_physicsLayerSetting;

		JPH::PhysicsSystem m_physicsSystem;

		PhysicsBodyCreator m_bodyCreator;

		bool m_isInitialized;

		bool m_isJoltTypeRegistered;

		bool m_isDisableDebugDraw;
	};
}