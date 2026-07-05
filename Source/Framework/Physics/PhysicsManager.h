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
		
		void ClearFrame                     ();
		void CollectPhysicsDebugDrawCommands();

		void TogglePhysicsDebugDraw();

		const auto& GetREFTempAllocator      () const { return m_tempAllocator; }
		const auto& GetREFPhysicsLayerSetting() const { return m_physicsLayerSetting; }
		const auto& GetREFPhysicsSystem      () const { return m_physicsSystem; }

		auto& GetMutableREFPhysicsSystem() { return m_physicsSystem; }

		std::weak_ptr<PhysicsDebugRenderer> GetVALDebugRenderer() const { return m_debugRenderer; }

		bool GetVALIsDisableDebugDraw() const { return m_isDisableDebugDraw; }

	private:

		bool SetupJoltCore();
		bool SetupSystem  ();
		
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
		void Release();

		static constexpr JPH::uint k_maxBodyCount = 1024U;

		static constexpr JPH::uint k_bodyMutexCount = 0U;

		static constexpr JPH::uint k_maxBodyPairCount = 1024U;

		static constexpr JPH::uint k_maxContactConstraintCount = 1024U;

		static constexpr uint32_t k_tempAllocatorSizeMB = 10U;
		static constexpr uint32_t k_kiloBytePerMB       = 1024U;
		static constexpr uint32_t k_bytePerKB           = 1024U;

		const std::filesystem::path k_configFileIOPath = "Asset/Data/CONFIG/Physics/PhysicsCONFIG.json";

		std::unique_ptr<JPH::Factory> m_factory;

		std::shared_ptr<JPH::TempAllocatorImpl> m_tempAllocator;

		std::shared_ptr<PhysicsLayerSetting> m_physicsLayerSetting;
		
		std::shared_ptr<PhysicsDebugRenderer> m_debugRenderer;

		JPH::PhysicsSystem m_physicsSystem;

		bool m_isInitialized;

		bool m_isJoltTypeRegistered;

		bool m_isDisableDebugDraw;
	};
}