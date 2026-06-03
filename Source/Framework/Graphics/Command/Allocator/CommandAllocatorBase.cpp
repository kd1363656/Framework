#include "CommandAllocatorBase.h"

FWK::Graphics::CommandAllocatorBase::CommandAllocatorBase(const D3D12_COMMAND_LIST_TYPE a_createCommandListType) : 
	k_createCommandListType(a_createCommandListType),

	m_commandAllocator(nullptr),

	m_submittedFenceValue(Constant::k_unusedFenceValue)
{}
FWK::Graphics::CommandAllocatorBase::~CommandAllocatorBase() = default;

bool FWK::Graphics::CommandAllocatorBase::Create(const Device& a_device)
{
	const auto& l_device = a_device.GetREFDevice();

	// デバイスが存在しないなら作成できないのでreturn
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_device, "デバイスの作成に失敗しており、コマンドアロケータの作成に失敗しました。", false);


	// コマンドアロケータを作成する関数
	// CreateCommandAllocator(このコマンドアロケータが記録対象とするコマンドリストの種類(※注意 : 作成するコマンドリストの種類と合わせる必要がある)、
	//						  受け取りたいCOMインターフェース型のID、
	//					      作成結果のポインタを書き込むアドレス);
	auto l_hr = l_device->CreateCommandAllocator(k_createCommandListType, IID_PPV_ARGS(m_commandAllocator.ReleaseAndGetAddressOf()));

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(FAILED(l_hr), "コマンドアロケータの作成に失敗しました。", false);

	return true;
}

void FWK::Graphics::CommandAllocatorBase::Reset() const
{
	FWK_ASSERT_RETURN_IF_FAILED(!m_commandAllocator, "コマンドアロケータの作成に失敗しており、リセット処理に失敗しました。");

	// コマンドアロケータを再利用できる状態に戻す関数
	// Reset();
	m_commandAllocator->Reset();
}