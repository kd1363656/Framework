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

	private:

		bool SetupJoltCore     ();
		bool SetupPhysicsSystem();

		void Release();

		static constexpr JPH::uint k_maxBodyCount = 1024U;

		static constexpr JPH::uint k_bodyMutexCount = 0U;

		static constexpr JPH::uint k_maxBodyPairCount = 1024U;

		static constexpr JPH::uint k_maxContactConstraintCount = 1024U;

		static constexpr uint32_t k_tempAllocatorSizeMB = 10U;
		static constexpr uint32_t k_kiloBytePerMB       = 1024U;
		static constexpr uint32_t k_bytePerKB           = 1024U;

		static constexpr uint32_t k_maxPhysicsJobCount     = 2048U;
		static constexpr uint32_t k_maxPhysicsBarrierCount = 8U;
		static constexpr uint32_t k_mainThreadCount        = 1U;
		static constexpr uint32_t k_minHardwareThreadCount = 1U;

		static constexpr int k_fallbackWorkerThreadCount = 1;
		
		static constexpr int k_collisionStepCount = 1;

		std::unique_ptr<JPH::Factory> m_factory;

		std::unique_ptr<JPH::TempAllocatorImpl> m_tempAllocator;

		std::unique_ptr<JPH::JobSystemThreadPool> m_jobSystem;

		PhysicsLayerSetting m_physicsLayerSetting;

		JPH::PhysicsSystem m_physicsSystem;

		bool m_isInitialized;

		bool m_isJoltTypeRegistered;

		bool m_isDisableDebugDraw;
	};
}