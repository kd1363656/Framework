#pragma once

namespace FWK::Graphics
{
	template <D3D12_COMMAND_LIST_TYPE CommandType>
	class CommandList
	{
	public:

		 CommandList() = default;
		~CommandList() = default;

		bool Create(const Device& a_device)
		{
			const auto& l_device = a_device.GetREFDevice();

			FWK_ASSERT_RETURN_VALUE_IF(!l_device, "デバイスの作成に失敗しており、コマンドリストの作成に失敗しました。", false);

			// 閉じた状態のコマンドリストを作成する関数(命令を書き込む前にResetして記録可能状態にする必要がある)
			// CreateCommandList(このコマンドリストを作成するGPUノード指定値、
			//                   このコマンドリストの種類、
			//					 コマンドリスト作成時の追加オプション、
			//					 受け取りたいCOMインターフェース型のID、
			//					 作成結果のポインタを書き込むアドレス);
			auto l_hr = l_device->CreateCommandList1(Constant::k_defaultGPUNodeMask,
													 CommandType,
													 D3D12_COMMAND_LIST_FLAG_NONE,
													 IID_PPV_ARGS(m_commandList.ReleaseAndGetAddressOf()));

			FWK_ASSERT_RETURN_VALUE_IF(FAILED(l_hr), "コマンドリストの作成に失敗しました。", false);

			return true;
		}

		void Reset(const CommandAllocator<CommandType>& a_commandAllocator)
		{
			FWK_ASSERT_RETURN_IF(!m_commandList, "コマンドリストの作成に失敗しており、コマンドリストのリセットに失敗しました。");

			// 自身のコマンドリストタイプと一致しなければreturn
			FWK_ASSERT_RETURN_IF(a_commandAllocator.GetVALCreateCommandListType() != CommandType, "コマンドアロケータのコマンドリストタイプと一致しないため、コマンドリストのリセットに失敗しました。");

			const auto& l_d3dCommandAllocator = a_commandAllocator.GetREFCommandAllocator();

			FWK_ASSERT_RETURN_IF(!l_d3dCommandAllocator, "コマンドアロケータの作成に失敗しており、コマンドリストのリセットに失敗しました。");

			// コマンドリストを再び記録できる状態に戻す関数
			// Reset(使用していたコマンドアロケータ、
			//		 最初に設定するパイプラインステート);
			auto l_hr = m_commandList->Reset(l_d3dCommandAllocator.Get(), nullptr);

			FWK_ASSERT_RETURN_IF(FAILED(l_hr), "コマンドリストのリセット処理に失敗しました。");
		}

		void Close() const
		{
			FWK_ASSERT_RETURN_IF(!m_commandList, "コマンドリストの作成に失敗しており、コマンドリストのクローズ処理に失敗しました。");

			// コマンドリストへの命令記録を終了するクラス
			// ※注意 : もしCloseをしなければコマンドキューのExecute処理を行うことができない
			// Close();
			auto l_hr = m_commandList->Close();

			FWK_ASSERT_RETURN_IF(FAILED(l_hr), "コマンドリストのクローズ処理に失敗しました。");
		}

		const auto& GetREFCommandList() const { return m_commandList; }

		auto GetVALCreateCommandListType() const { return k_createCommandListType; }

	private:

		TypeAlias::ComPtr<ID3D12GraphicsCommandList6> m_commandList = nullptr;
	};
}