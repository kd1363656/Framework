#pragma once

namespace FWK::Graphics
{
	template <D3D12_COMMAND_LIST_TYPE CommandType>
	class CommandListBase
	{
	public:

		         CommandListBase() = default;
		virtual ~CommandListBase() = default;

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
			auto l_hr = l_device->CreateCommandList1(UploadBuffer::k_defaultGPUNodeMask,
													 CommandType,
													 D3D12_COMMAND_LIST_FLAG_NONE,
													 IID_PPV_ARGS(m_commandList.ReleaseAndGetAddressOf()));

			FWK_ASSERT_RETURN_VALUE_IF(FAILED(l_hr), "コマンドリストの作成に失敗しました。", false);

			return true;
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

		void TransitionResourceBarrier(const TypeAlias::ComPtr<ID3D12Resource2>& a_resource, const D3D12_RESOURCE_STATES a_beforeState, const D3D12_RESOURCE_STATES a_afterState) const
		{
			FWK_ASSERT_RETURN_IF(!a_resource, "状態遷移予定のリソースが無効になっているため、リソースの遷移に失敗しました。");

			if (a_beforeState == a_afterState) { return; }

			const auto& l_directCommandList = GetREFCommandList();

			FWK_ASSERT_RETURN_IF(!l_directCommandList, "ダイレクトコマンドリストが作成されておらず、リソースの遷移に失敗しました。");

			// D3D12_RESOURCE_BARRIER構造体についての説明(CD3DX12_RESOURCE_BARRIER::Transition内部で使用)
			// Type                   : このバリアがどういうバリアであるかを指定
			// Flags                  : バリアの特別な追加設定
			// Transition.pResource   : 状態を切り替える対象のGPUリソース
			// Transition.StateBefore : 切り替える前のリソース状態
			// Transition.StateAfter  : 切り替えた後のリソース状態
			// Transition.Subresource : どのサブリソースを遷移対象にするか
			const auto& l_barrier = CD3DX12_RESOURCE_BARRIER::Transition(a_resource.Get(), a_beforeState, a_afterState);

			// リソースバリアを転送
			// ResourceBarrier(送るバリア数、
			//				   バリア情報の先頭アドレス)
			l_directCommandList->ResourceBarrier(k_singleSetupBarrierNUM, &l_barrier);
		}

		const auto& GetREFCommandList() const { return m_commandList; }

	protected:

		virtual void Reset(const CommandAllocator<CommandType>& a_commandAllocator)
		{
			FWK_ASSERT_RETURN_IF(!m_commandList, "コマンドリストの作成に失敗しており、コマンドリストのリセットに失敗しました。");

			const auto& l_d3dCommandAllocator = a_commandAllocator.GetREFCommandAllocator();

			FWK_ASSERT_RETURN_IF(!l_d3dCommandAllocator, "コマンドアロケータの作成に失敗しており、コマンドリストのリセットに失敗しました。");

			// コマンドリストを再び記録できる状態に戻す関数
			// Reset(使用していたコマンドアロケータ、
			//		 最初に設定するパイプラインステート);
			auto l_hr = m_commandList->Reset(l_d3dCommandAllocator.Get(), nullptr);

			FWK_ASSERT_RETURN_IF(FAILED(l_hr), "コマンドリストのリセット処理に失敗しました。");
		}

		static constexpr UINT GetVALSingleSetupBarrierNum() { return k_singleSetupBarrierNUM; }

	private:

		static constexpr UINT k_singleSetupBarrierNUM = 1U;

		TypeAlias::ComPtr<ID3D12GraphicsCommandList6> m_commandList = nullptr;
	};
}