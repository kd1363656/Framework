#include "CommandQueueBase.h"

FWK::Graphics::CommandQueueBase::CommandQueueBase(const D3D12_COMMAND_LIST_TYPE a_createCommandListType) : 
	k_createCommandListType(a_createCommandListType),

	m_commandQueue(nullptr),

	m_fence()
{}
FWK::Graphics::CommandQueueBase::~CommandQueueBase() 
{
	// GPUと完全同期をとってDirectX12のデバイスが
	// Releaseされてもいい状態にする
	WaitForGPUIdleIfNeeded();
}

bool FWK::Graphics::CommandQueueBase::Create(const Device& a_device)
{
	FWK_ASSERT_RETURN_VALUE_IF(!CreateCommandQueue(a_device), "コマンドキューの作成に失敗しました。", false)
	FWK_ASSERT_RETURN_VALUE_IF(!CreateFence(a_device),		  "フェンスの作成に失敗しました。",	      false)

	return true;
}

void FWK::Graphics::CommandQueueBase::WaitForFenceValueIfNeeded(const UINT64& a_waitFenceValue)
{
	// 待つ必要があればWaitする
	m_fence.WaitForFenceValueIfNeeded(a_waitFenceValue);
}
void FWK::Graphics::CommandQueueBase::WaitForGPUIdleIfNeeded()
{
	const auto& l_fence = m_fence.GetREFFence();

	// フェンスが存在しなければGPU完了確認はできない
	FWK_ASSERT_RETURN_IF(!l_fence,		  "フェンスの作成に失敗しておりコマンドアロケータの使用可能かどうかの選定に失敗しました。")
	FWK_ASSERT_RETURN_IF(!m_commandQueue, "コマンドキューが作成されておらず、GPUとの同期が取れません。")

	// 今回の待機用に新しいフェンス値を発行する
	// 同じ値を使いまわすとどこまでの処理完了を待っているのか分からなくなるため
	const auto& l_incrementedFenceValue = FetchREFLastSignaledFenceValue() + k_incrementFenceValue;

	m_fence.SetLastSignaledFenceValue(l_incrementedFenceValue);

	// コマンドキューに対して「命令したGPU処理が終わったら、m_fenceの値をl_targetFenceValueに更新してください」
	// と命令をする関数
	// Signal(更新対象のフェンスオブジェクト、
	//		  GPU完了時に設定するフェンス値);

	auto l_hr = m_commandQueue->Signal(l_fence.Get(), l_incrementedFenceValue);

	// Signal命令に失敗したらreturn
	FWK_ASSERT_RETURN_IF(FAILED(l_hr), "コマンドキューへのフェンスシグナルに失敗しました。")

	WaitForFenceValueIfNeeded(l_incrementedFenceValue);
}

void FWK::Graphics::CommandQueueBase::EnsureAllocatorAvailable(const CommandAllocatorBase& a_commandAllocator)
{
	// このコマンドアロケータの前回送信分が完了していれば待機不要
	// 未完了なら安全に再利用できるまで待機する
	WaitForFenceValueIfNeeded(a_commandAllocator.GetREFSubmittedFenceValue());
}

void FWK::Graphics::CommandQueueBase::ExecuteCommandLists(const CommandListBase& a_commandList) const
{
	const auto& l_commandQueue = GetREFCommandQueue             ();
	const auto& l_commandList  = a_commandList.GetREFCommandList();

	FWK_ASSERT_RETURN_IF(!l_commandQueue, "コマンドキューが作成されておらず、コマンド実行処理が行えませんでした")
	FWK_ASSERT_RETURN_IF(!l_commandList,  "コマンドリストが作成されておらず、コマンド実行処理が行えませんでした")

	// このキューと違うコマンドリストタイプならreturn
	FWK_ASSERT_RETURN_IF(k_createCommandListType != a_commandList.GetVALCreateCommandListType(), "コマンドリストとコマンドキューのコマンドリストタイプが違います、コマンド実行処理が行えませんでした")

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

void FWK::Graphics::CommandQueueBase::SignalAndTrackAllocator(CommandAllocatorBase& a_commandAllocator)
{
	const auto& l_fence = m_fence.GetREFFence();

	FWK_ASSERT_RETURN_IF(!l_fence,		  "フェンスが作成されておらず、GPUとの同期処理が行えませんでした。")
	FWK_ASSERT_RETURN_IF(!m_commandQueue, "ダイレクトコマンドキューが作成されておらず、GPUとの同期処理が行えませんでした。")

	const auto& l_updatedFenceValue = FetchREFLastSignaledFenceValue() + k_incrementFenceValue;

	// "FenceValue"を進めて、このフレームの完了目標として保存
	m_fence.SetLastSignaledFenceValue(l_updatedFenceValue);

	// ※重要
	// 更新したフェンス値を持たせて置く、こうすることで次のフレームでフェンス値を超えていない場合
	// GPUとの同期をとらなくていいためCPUとGPUの並列処理性を発揮することができる
	a_commandAllocator.SetSubmittedFenceValue(l_updatedFenceValue);

	// コマンドキュー内でこの位置までの命令が実行完了したら
	// フェンス値をGetFenceValueに更新する命令をGPUに追加
	m_commandQueue->Signal(l_fence.Get(), l_updatedFenceValue);
}

bool FWK::Graphics::CommandQueueBase::IsFenceValueCompleted(const UINT64& a_fenceValue) const
{
	return m_fence.IsFenceValueCompleted(a_fenceValue);
}

const UINT64& FWK::Graphics::CommandQueueBase::FetchREFLastSignaledFenceValue() const
{
	return m_fence.GetREFLastSignaledFenceValue();
}

UINT64 FWK::Graphics::CommandQueueBase::FetchVALCompletedFenceValue() const
{
	return m_fence.FetchVALCompletedFenceValue();
}

bool FWK::Graphics::CommandQueueBase::CreateCommandQueue(const Device& a_device)
{
	const auto& l_device = a_device.GetREFDevice();

	// デバイスが存在しないなら作成できないのでreturn
	FWK_ASSERT_RETURN_VALUE_IF(!l_device, "デバイスの作成に失敗しており、コマンドキューの作成が出来ませんでした。", false)

	// コマンドキュー作成時に必要な設定構造体
	// この構造体に「どんな種類のキューを作るか」を設定してからCreateCommandQueueに渡す
	D3D12_COMMAND_QUEUE_DESC l_desc = {};

	// このコマンドキューで実行するコマンドリストの種類を指定する
	// ※注意 : この値は実行するコマンドリストのTypeと一致している必要がある
	l_desc.Type = k_createCommandListType;

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

	FWK_ASSERT_RETURN_VALUE_IF(FAILED(l_hr), "コマンドキューの作成に失敗しました。", false)

	return true;
}
bool FWK::Graphics::CommandQueueBase::CreateFence(const Device& a_device)
{
	return m_fence.Create(a_device);
}