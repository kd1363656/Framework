#pragma once

namespace FWK::Graphics
{
	class CommandAllocatorBase
	{
	public:

		explicit CommandAllocatorBase(const D3D12_COMMAND_LIST_TYPE a_createCommandListType);
		virtual ~CommandAllocatorBase();

		bool Create(const Device& a_device);

		void Reset() const;

		void SetSubmittedFenceValue(const auto& a_set) { m_submittedFenceValue = a_set; }

		const auto& GetREFCommandAllocator() const { return m_commandAllocator; }

		const auto& GetREFSubmittedFenceValue() const { return m_submittedFenceValue; }

		auto GetVALCreateCommandListType() const { return k_createCommandListType; }

	private:

		const D3D12_COMMAND_LIST_TYPE k_createCommandListType;

		TypeAlias::ComPtr<ID3D12CommandAllocator> m_commandAllocator;

		UINT64 m_submittedFenceValue;
	};
}