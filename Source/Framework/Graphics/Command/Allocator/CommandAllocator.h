#pragma once

namespace FWK::Graphics
{
	template <D3D12_COMMAND_LIST_TYPE CommandType>
	class CommandAllocator final
	{
	public:

		 CommandAllocator() = default;
		~CommandAllocator() = default;

		bool Create(const Device& a_device)
		{
			const auto& l_device = a_device.GetREFDevice();

			// デバイスが存在しないなら作成できないのでreturn
			FWK_ASSERT_RETURN_VALUE_IF(!l_device, "デバイスの作成に失敗しており、コマンドアロケータの作成に失敗しました。", false);

			// コマンドアロケータを作成する関数
			// CreateCommandAllocator(このコマンドアロケータが記録対象とするコマンドリストの種類(※注意 : 作成するコマンドリストの種類と合わせる必要がある)、
			//						  受け取りたいCOMインターフェース型のID、
			//					      作成結果のポインタを書き込むアドレス);
			auto l_hr = l_device->CreateCommandAllocator(CommandType, IID_PPV_ARGS(m_commandAllocator.ReleaseAndGetAddressOf()));

			FWK_ASSERT_RETURN_VALUE_IF(FAILED(l_hr), "コマンドアロケータの作成に失敗しました。", false);

			return true;
		}

		void Reset() const
		{
			FWK_ASSERT_RETURN_IF(!m_commandAllocator, "コマンドアロケータの作成に失敗しており、リセット処理に失敗しました。");

			// コマンドアロケータを再利用できる状態に戻す関数
			// Reset();
			auto l_hr = m_commandAllocator->Reset();

			FWK_ASSERT_RETURN_IF(FAILED(l_hr), "コマンドアロケータのリセット処理に失敗しました。");
		}

		void SetSubmittedFenceValue(const auto& a_set) { m_submittedFenceValue = a_set; }

		const auto& GetREFCommandAllocator() const { return m_commandAllocator; }

		const auto& GetREFSubmittedFenceValue() const { return m_submittedFenceValue; }

	private:

		TypeAlias::ComPtr<ID3D12CommandAllocator> m_commandAllocator = nullptr;

		UINT64 m_submittedFenceValue = Constant::k_unusedFenceValue;
	};
}