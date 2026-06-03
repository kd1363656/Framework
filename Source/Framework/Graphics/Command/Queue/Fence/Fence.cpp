#include "Fence.h"

FWK::Graphics::Fence::Fence() : 
	m_fence(nullptr),

	m_event(nullptr),

	m_lastSignaledFenceValue(Constant::k_unusedFenceValue)
{}
FWK::Graphics::Fence::~Fence() 
{
	// イベントハンドルが作成されていなければreturn
	if (!m_event) { return; }

	// イベントハンドルを開放
	CloseHandle(m_event);

	m_event = nullptr;
}

bool FWK::Graphics::Fence::Create(const Device& a_device)
{
	const auto& l_device = a_device.GetREFDevice();

	// デバイスが存在しなければフェンスを作成することはできないのでreturn
	FWK_ASSERT_RETURN_VALUE_IF(!l_device, "デバイスが作成されておらず、フェンスの作成に失敗しました。", false)

	// フェンスを作成する関数
	// CreateFence(フェンスの初期値、
	//			   フェンス作成時の追加設定、
	//			   受け取りたいCOMインターフェース型のID、
	//			   作成結果のポインタを書き込むアドレス);
	auto l_hr = l_device->CreateFence(m_lastSignaledFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.ReleaseAndGetAddressOf()));

	FWK_ASSERT_RETURN_VALUE_IF(FAILED(l_hr), "フェンスの作成に失敗しました。", false)

	// CPU側が「GPUの処理完了を待つ」ときに使うイベントオブジェクトを作成する
	// CreateEvent(セキュリティ属性、
	//			   manualResetにするかどうか(FALSEなので自動リセットイベント、一度シグナル状態になって待機が解除されたら、自動で非シグナル状態に戻る、
	//			   作成直後にシグナル状態にするかどうか、
	//			   イベント名);
	m_event = CreateEvent(nullptr,
						  FALSE,
						  FALSE,
						  nullptr);

	FWK_ASSERT_RETURN_VALUE_IF(!m_event, "イベントの作成に失敗しました。", false)

	return true;
}

void FWK::Graphics::Fence::WaitForFenceValueIfNeeded(const UINT64& a_fenceValue)
{
	// フェンスが存在しなければGPU完了確認はできない
	FWK_ASSERT_RETURN_IF(!m_fence, "フェンスの作成に失敗しており、フェンス値まで待つかどうかの判定に失敗しました。")

	// GPUの完了値が、引数で渡された目標フェンス値以上なら、
	// その地点までの処理は完了済みと見なせるのでreturn
	if (const auto& l_completedValue = m_fence->GetCompletedValue();
		l_completedValue >= a_fenceValue) 
	{
		return; 
	}

	// GPUがa_fenceValueに到達したらm_fenceEventを通知状態にする関数
	// SetEventOnCompletion(CPUが待ちたい目標のフェンス値、
	//						GPUが完了通知を受け取るイベント);
	const auto l_hr = m_fence->SetEventOnCompletion(a_fenceValue, m_event);

	FWK_ASSERT_RETURN_IF(FAILED(l_hr), "フェンス完了イベントの設定に失敗しました。")

	// m_fenceEventが通知されるまでCPUを待機させる
	// INFINITEは時間制限なしで待つ指定
	// WaitForSingleObject(待機対象のイベント、
	//					   待機時間);
	const auto l_waitResult = WaitForSingleObject(m_event, INFINITE);

	FWK_ASSERT_RETURN_IF(l_waitResult != WAIT_OBJECT_0, "フェンスの待機処理に失敗しました。")
}

bool FWK::Graphics::Fence::IsFenceValueCompleted(const UINT64& a_fenceValue) const
{
	// フェンス値が未使用の値なら、フェンス完了確認は不要なのでtrueを返す
	if (a_fenceValue == Constant::k_unusedFenceValue) { return true; }

	// フェンスが存在しなければreturn
	FWK_ASSERT_RETURN_VALUE_IF(!m_event, "フェンスが作成されておらず、フェンス完了確認処理に失敗しました。", false)

	return m_fence->GetCompletedValue() >= a_fenceValue;
}

UINT64 FWK::Graphics::Fence::FetchVALCompletedFenceValue() const
{
	FWK_ASSERT_RETURN_VALUE_IF(!m_fence, "フェンスの作成に失敗しており完了フェンス値の取得に失敗しました。", Constant::k_unusedFenceValue)

	return m_fence->GetCompletedValue();
}