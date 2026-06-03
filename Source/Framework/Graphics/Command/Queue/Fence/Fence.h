#pragma once

namespace FWK::Graphics
{
	// コピーやムーブを許可すると、複数インスタンスが同じイベントハンドルをCloseHandleする危険があるため禁止
	class Fence final
	{
	public:

		 Fence();
		~Fence();

		Fence(const Fence&)			  = delete;
		Fence(		Fence&&) noexcept = delete;
		
		Fence& operator=(const Fence&)		     = delete;
		Fence& operator=(	   Fence&&) noexcept = delete;

		bool Create(const Device& a_device);

		void WaitForFenceValueIfNeeded(const UINT64& a_fenceValue);

		bool IsFenceValueCompleted(const UINT64& a_fenceValue) const;

		void SetLastSignaledFenceValue(const auto& a_set) { m_lastSignaledFenceValue = a_set; }

		UINT64 FetchVALCompletedFenceValue() const;

		const auto& GetREFLastSignaledFenceValue() const { return m_lastSignaledFenceValue; }

		const auto& GetREFFence() const { return m_fence; }

	private:

		TypeAlias::ComPtr<ID3D12Fence1> m_fence;

		HANDLE m_event;

		UINT64 m_lastSignaledFenceValue;
	};
}