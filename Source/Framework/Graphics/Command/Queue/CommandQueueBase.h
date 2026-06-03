#pragma once

namespace FWK::Graphics
{
	class CommandAllocatorBase;
	class CommandListBase;
}

namespace FWK::Graphics
{
	class CommandQueueBase
	{
	public:
		
		explicit  CommandQueueBase(const D3D12_COMMAND_LIST_TYPE a_createCommandListType);
		virtual  ~CommandQueueBase();

		bool Create(const Device& a_device);

		void WaitForFenceValueIfNeeded(const UINT64& a_waitFenceValue);
		void WaitForGPUIdleIfNeeded	  ();

		void EnsureAllocatorAvailable(const CommandAllocatorBase& a_commandAllocator);

		void ExecuteCommandLists(const CommandListBase& a_commandList) const;

		void SignalAndTrackAllocator(CommandAllocatorBase& a_commandAllocator);

		bool IsFenceValueCompleted(const UINT64& a_fenceValue) const;

		const UINT64& FetchREFLastSignaledFenceValue() const;

		UINT64 FetchVALCompletedFenceValue() const;

		const auto& GetREFCommandQueue() const { return m_commandQueue; }

		D3D12_COMMAND_LIST_TYPE GetVALCreateCommandListType() const { return k_createCommandListType; }

	private:

		bool CreateCommandQueue(const Device& a_device);
		bool CreateFence	   (const Device& a_device);

		static constexpr UINT64 k_incrementFenceValue = 1ULL;

		static constexpr UINT k_executeListNum = 1U;

		const D3D12_COMMAND_LIST_TYPE k_createCommandListType;

		TypeAlias::ComPtr<ID3D12CommandQueue> m_commandQueue;

		Fence m_fence;
	};
}