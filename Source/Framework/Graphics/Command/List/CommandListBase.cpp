#include "CommandListBase.h"

FWK::Graphics::CommandListBase::CommandListBase(const D3D12_COMMAND_LIST_TYPE a_createCommandListType) :
	k_createCommandListType(a_createCommandListType),

	m_commandList(nullptr)
{}
FWK::Graphics::CommandListBase::~CommandListBase() = default;

bool FWK::Graphics::CommandListBase::Create(const Device& a_device)
{
	const auto& l_device = a_device.GetREFDevice();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_device, "デバイスの作成に失敗しており、コマンドリストの作成に失敗しました。", false);

	// 閉じた状態のコマンドリストを作成する関数(命令を書き込む前にResetして記録可能状態にする必要がある)
	// CreateCommandList(このコマンドリストを作成するGPUノード指定値、
	//                   このコマンドリストの種類、
	//					 コマンドリスト作成時の追加オプション、
	//					 受け取りたいCOMインターフェース型のID、
	//					 作成結果のポインタを書き込むアドレス);
	auto l_hr = l_device->CreateCommandList1(Constant::k_defaultGPUNodeMask,
											 k_createCommandListType,
											 D3D12_COMMAND_LIST_FLAG_NONE,
											 IID_PPV_ARGS(m_commandList.ReleaseAndGetAddressOf()));

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(FAILED(l_hr), "コマンドリストの作成に失敗しました。", false);

	return true;
}

void FWK::Graphics::CommandListBase::Reset(const CommandAllocatorBase& a_commandAllocator)
{
	FWK_ASSERT_RETURN_IF_FAILED(!m_commandList, "コマンドリストの作成に失敗しており、コマンドリストのリセットに失敗しました。");

	// 自身のコマンドリストタイプと一致しなければreturn
	FWK_ASSERT_RETURN_IF_FAILED(a_commandAllocator.GetVALCreateCommandListType() != k_createCommandListType, "コマンドアロケータのコマンドリストタイプと一致しないため、コマンドリストのリセットに失敗しました。");

	const auto& l_d3dCommandAllocator = a_commandAllocator.GetREFCommandAllocator();

	FWK_ASSERT_RETURN_IF_FAILED(!l_d3dCommandAllocator, "コマンドアロケータの作成に失敗しており、コマンドリストのリセットに失敗しました。");

	// コマンドリストを再び記録できる状態に戻す関数
	// Reset(使用していたコマンドアロケータ、
	//		 最初に設定するパイプラインステート);
	m_commandList->Reset(l_d3dCommandAllocator.Get(), nullptr);
}

void FWK::Graphics::CommandListBase::Close() const
{
	FWK_ASSERT_RETURN_IF_FAILED(!m_commandList, "コマンドリストの作成に失敗しており、コマンドリストのクローズ処理に失敗しました。");

	// コマンドリストへの命令記録を終了するクラス
	// ※注意 : もしCloseをしなければコマンドキューのExecute処理を行うことができない
	// Close();
	m_commandList->Close();
}