#pragma once

namespace FWK::Graphics
{
	template <D3D12_COMMAND_LIST_TYPE CommandType>
	class CommandQueue final
	{
	public:

		 CommandQueue() = default;
		~CommandQueue()
		{
			WaitForGPUIdleIfNeeded();
		}

		bool Create(const Device& a_device)
		{
			FWK_ASSERT_RETURN_VALUE_IF(!CreateCommandQueue(a_device), "コマンドキューの作成に失敗しました。", false);
			FWK_ASSERT_RETURN_VALUE_IF(!CreateFence(a_device),		 "フェンスの作成に失敗しました。",	      false);

			return true;	
		}

		void WaitForFenceValueIfNeeded(const UINT64& a_waitFenceValue)
		{
			// CommandAllocatorの再利用や終了処理など、
			// CPU側がGPUの完了を待つ必要がある場合に使用する
			m_fence.WaitForFenceValueIfNeeded(a_waitFenceValue);
		}
		void WaitForGPUIdleIfNeeded()
		{
			const auto& l_fence = m_fence.GetREFFence();

			// フェンスが存在しなければGPU完了確認はできない
			FWK_ASSERT_RETURN_IF(!l_fence,		  "フェンスの作成に失敗しておりコマンドアロケータの使用可能かどうかの選定に失敗しました。");
			FWK_ASSERT_RETURN_IF(!m_commandQueue, "コマンドキューが作成されておらず、GPUとの同期に失敗しました。");

			// 今回の待機用に新しいフェンス値を発行する
			// 同じ値を使いまわすとどこまでの処理完了を待っているのか分からなくなるため
			const auto& l_incrementedFenceValue = FetchREFLastSignaledFenceValue() + k_incrementFenceValue;

			m_fence.SetLastSignaledFenceValue(l_incrementedFenceValue);

			// コマンドキューに対して「命令したGPU処理が終わったら、m_fenceの値をl_targetFenceValueに更新してください」と命令をする関数
			// Signal(更新対象のフェンスオブジェクト、
			//		  GPU完了時に設定するフェンス値);
			auto l_hr = m_commandQueue->Signal(l_fence.Get(), l_incrementedFenceValue);

			// Signal命令に失敗したらreturn
			FWK_ASSERT_RETURN_IF(FAILED(l_hr), "コマンドキューへのフェンスシグナルに失敗しました。");

			WaitForFenceValueIfNeeded(l_incrementedFenceValue);
		}

		// このコマンドキューを、別のCommandQueueがSignalしたFence値までGPU上で待機させる
		// CPUスレッドは停止しない
		template <D3D12_COMMAND_LIST_TYPE WaitCommandType>
		bool Wait(const CommandQueue<WaitCommandType>& a_waitCommandQueue, const UINT64& a_waitFenceValue) const
		{
			// 未使用フェンス値なら、待機対象がないことを表す
			if (a_waitCommandQueue == Constant::k_unusedFenceValue) { return true; }

			const auto& l_waitFence = a_waitCommandQueue.GetREFFence().GetREFence();

			FWK_ASSERT_RETURN_VALUE_IF(!m_commandQueue, "待機命令を登録するCommandQueueが作成されていません。", false);
			FWK_ASSERT_RETURN_VALUE_IF(!l_waitFence,    "待機対象CommandQueueのFenceが作成されていません。",    false);

			// 使用例
			// DirectQueue.Wait(ComputeQueue, ComputeFenceValue);
			// ComputeShaderの書き込み完了後に、
			// DirectQueueの描画処理を開始する
			// 阪大方向に待機させれば、前回の描画がBufferを読み終えるまで次回のCompute書き込みを開始しないようにできる
			const auto l_hr = m_commandQueue->Wait(l_waitFence.Get(), a_waitFenceValue);

			FWK_ASSERT_RETURN_VALUE_IF(FAILED(l_hr), "CommandQueue間のGPU待機命令登録に失敗しました。", false);

			return true;
		}

		void EnsureAllocatorAvailable(const CommandAllocator<CommandType>& a_commandAllocator)
		{
			// このコマンドアロケータの前回送信分が完了していれば待機不要
			// 未完了なら安全に再利用できるまで待機する
			WaitForFenceValueIfNeeded(a_commandAllocator.GetREFSubmittedFenceValue());
		}

		void ExecuteCommandLists(const CommandListBase<CommandType>& a_commandList) const
		{
			const auto& l_commandQueue = GetREFCommandQueue             ();
			const auto& l_commandList  = a_commandList.GetREFCommandList();

			FWK_ASSERT_RETURN_IF(!l_commandQueue, "コマンドキューが作成されておらず、コマンド実行処理に失敗しました。");
			FWK_ASSERT_RETURN_IF(!l_commandList,  "コマンドリストが作成されておらず、コマンド実行処理に失敗しました。");

			// ExecuteCommandLists()はID3D12CommandList*の配列を受け取るため、
			// 1個だけ実行する場合でも配列にして渡す必要がある
			ID3D12CommandList* l_list[] = 
			{
				l_commandList.Get() 
			};

			// ExecuteCommandLists(実行するコマンドリストの数、
			//					   コマンドリスト配列の先頭アドレス)
			// ※注意 : ExecuteCommandListをする前にコマンドリストはClose()関数を呼び出す必要がある
			l_commandQueue->ExecuteCommandLists(k_executeListNum, l_list);
		}

		void SignalAndTrackAllocator(CommandAllocator<CommandType>& a_commandAllocator)
		{
			const auto& l_fence = m_fence.GetREFFence();

			FWK_ASSERT_RETURN_IF(!l_fence,		  "フェンスが作成されておらず、GPUとの同期処理に失敗しました。");
			FWK_ASSERT_RETURN_IF(!m_commandQueue, "ダイレクトコマンドキューが作成されておらず、GPUとの同期処理に失敗しました。");

			const auto& l_updatedFenceValue = FetchREFLastSignaledFenceValue() + k_incrementFenceValue;

			// "FenceValue"を進めて、このフレームの完了目標として保存
			m_fence.SetLastSignaledFenceValue(l_updatedFenceValue);

			// ※重要
			// 更新したフェンス値を持たせて置く、こうすることで次のフレームでフェンス値を超えていない場合
			// GPUとの同期をとらなくていいためCPUとGPUの並列処理性を発揮することができる
			a_commandAllocator.SetSubmittedFenceValue(l_updatedFenceValue);

			// コマンドキュー内でこの位置までの命令が実行完了したら
			// フェンス値をGetFenceValueに更新する命令をGPUに追加
			auto l_hr = m_commandQueue->Signal(l_fence.Get(), l_updatedFenceValue);

			FWK_ASSERT_RETURN_IF(FAILED(l_hr), "GPUとの同期処理に失敗しました。");
		}

		bool IsFenceValueCompleted(const UINT64& a_fenceValue) const
		{
			return m_fence.IsFenceValueCompleted(a_fenceValue);
		}

		const UINT64& FetchREFLastSignaledFenceValue() const
		{
			return m_fence.GetREFLastSignaledFenceValue();
		}

		UINT64 FetchVALCompletedFenceValue() const
		{
			return m_fence.FetchVALCompletedFenceValue();
		}

		const auto& GetREFCommandQueue() const { return m_commandQueue; }

		const auto& GetREFFence() const { return m_fence; }

	private:

		bool CreateCommandQueue(const Device& a_device)
		{
			const auto& l_device = a_device.GetREFDevice();

			// デバイスが存在しないなら作成できないのでreturn
			FWK_ASSERT_RETURN_VALUE_IF(!l_device, "デバイスの作成に失敗しており、コマンドキューの作成に失敗しました。", false);

			// コマンドキュー作成時に必要な設定構造体
			// この構造体に「どんな種類のキューを作るか」を設定してからCreateCommandQueueに渡す
			D3D12_COMMAND_QUEUE_DESC l_desc = {};

			// このコマンドキューで実行するコマンドリストの種類を指定する
			// ※注意 : この値は実行するコマンドリストのTypeと一致している必要がある
			l_desc.Type = CommandType;

			// コマンドキューの優先度を指定する
			l_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
			
			// コマンドキューの追加オプションを指定する
			l_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

			// このコマンドキューをどのGPUノードで使用するかを指定する
			l_desc.NodeMask = Constant::k_defaultGPUNodeMask;

			// コマンドキューを作成する関数
			// CreateCommandQueue(コマンドキューの設定内容、
			//					  受け取りたいCOMインターフェース型のID、
			//					  作成結果のポインタを書き込むアドレス);
			auto l_hr = l_device->CreateCommandQueue(&l_desc, IID_PPV_ARGS(m_commandQueue.ReleaseAndGetAddressOf()));

			FWK_ASSERT_RETURN_VALUE_IF(FAILED(l_hr), "コマンドキューの作成に失敗しました。", false);

			return true;
		}
		bool CreateFence(const Device& a_device)
		{
			return m_fence.Create(a_device);
		}

		static constexpr UINT64 k_incrementFenceValue = 1ULL;

		static constexpr UINT k_executeListNum = 1U;

		TypeAlias::ComPtr<ID3D12CommandQueue> m_commandQueue;

		Fence m_fence;
	};
}