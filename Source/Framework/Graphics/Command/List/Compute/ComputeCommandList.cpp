#include "ComputeCommandList.h"

void FWK::Graphics::ComputeCommandList::Reset(const TypeAlias::ComputeCommandAllocator& a_computeCommandAllocator)
{
	DirectAndComputeCommandListBase::Reset(a_computeCommandAllocator);
}

void FWK::Graphics::ComputeCommandList::UAVResourceBarrier(const TypeAlias::ComPtr<ID3D12Resource2>& a_resource) const
{
	FWK_ASSERT_RETURN_IF(!a_resource, "UAVBarrierを設定するGPUResourceが無効です。");

	const auto& l_commandList = this->GetREFCommandList();

	FWK_ASSERT_RETURN_IF(!l_commandList, "CommandListが作成されておらず、UAV Barrierの設定に失敗しました。");

	// UAV用リソースバリアの作成
	const auto& l_resourceBarrier = CD3DX12_RESOURCE_BARRIER::UAV(a_resource.Get());

	l_commandList->ResourceBarrier(GetVALSingleSetupBarrierNum(), &l_resourceBarrier);
}

void FWK::Graphics::ComputeCommandList::SetupComputePipeline(const std::weak_ptr<ComputePipelineState>& a_pipelineState)
{
	DirectAndComputeCommandListBase::SetupPipeline(a_pipelineState);
}

void FWK::Graphics::ComputeCommandList::SetupConstantBufferView(const D3D12_GPU_VIRTUAL_ADDRESS& a_gpuVirtualAddress, const RootSignature& a_rootSignature, const Enum::RootParameterType a_rootParameterType) const
{
	const auto& l_computeCommandList = GetREFCommandList();

	FWK_ASSERT_RETURN_IF(!l_computeCommandList, "コンピュートコマンドリストが作成されておらず、定数バッファビュー設定に失敗しました。");

	const auto l_rootParameterIndex = a_rootSignature.FindVALRootParameterIndex(a_rootParameterType);

	FWK_ASSERT_RETURN_IF(l_rootParameterIndex == Constant::k_invalidRootParameterIndex, "パラメータインデックスが無効なため、コンピュート用定数バッファービュー設定に失敗しました。");

	// ComputePipeline用のRootCBVを設定する
	l_computeCommandList->SetComputeRootConstantBufferView(l_rootParameterIndex, a_gpuVirtualAddress);
}

void FWK::Graphics::ComputeCommandList::Dispatch(const UINT a_threadGroupCountX, const UINT a_threadGroupCountY, const UINT a_threadGroupCountZ) const
{
	const auto& l_computeCommandList = GetREFCommandList();

	FWK_ASSERT_RETURN_IF(!l_computeCommandList, "コンピュートコマンドリストが作成されておらず、Dispatchに失敗しました。");

	l_computeCommandList->Dispatch(a_threadGroupCountX, a_threadGroupCountY, a_threadGroupCountZ);
}

void FWK::Graphics::ComputeCommandList::SetupRootSignature(ID3D12GraphicsCommandList6& a_commandList, ID3D12RootSignature& a_rootSignature)
{
	a_commandList.SetComputeRootSignature(&a_rootSignature);
}

void FWK::Graphics::ComputeCommandList::SetupRoot32BitConstants(const RootSignature&          a_rootSignature, 
																const void*                   a_rootConstantData,
	                                                            const Enum::RootParameterType a_rootParameterType, 
	                                                            const UINT                    a_rootConstantCount,
	                                                            const UINT                    a_destinationOffset) const
{
	FWK_ASSERT_RETURN_IF(!a_rootConstantData,                               "Root32BitConstantsへ設定するデータが無効です。");
	FWK_ASSERT_RETURN_IF(a_rootConstantCount == k_invalidRootConstantCount, "Root32BitConstantsの設定数が0のため、設定に失敗しました。");

	const auto& l_computeCommandList = GetREFCommandList();

	FWK_ASSERT_RETURN_IF(!l_computeCommandList, "コンピュートコマンドリストが作成されておらず、Root32BitConstantsの設定に失敗しました。");

	const auto  l_rootParameterIndex = a_rootSignature.FindVALRootParameterIndex(a_rootParameterType);

	FWK_ASSERT_RETURN_IF(l_rootParameterIndex == Constant::k_invalidRootParameterIndex, "パラメータインデックスが無効なため、Root32BitConstantsの設定に失敗しました。");

	const auto& l_rootParameterRecordList = a_rootSignature.GetREFRootParameterRecordList();

	FWK_ASSERT_RETURN_IF(l_rootParameterIndex >= l_rootParameterRecordList.size(), "RootParameterIndexがRootParameterRecordListの範囲外です。");

	const auto& l_rootParameter = l_rootParameterRecordList[l_rootParameterIndex].m_rootParameter;

	FWK_ASSERT_RETURN_IF(l_rootParameter.ParameterType != D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS, "指定されたRootParameterTypeがRoot32BitConstantsではありません。");
	FWK_ASSERT_RETURN_IF(a_destinationOffset > l_rootParameter.Constants.Num32BitValues,             "Root32BitConstantsの書き込み開始位置が設定可能範囲を超えています。");

	// 先にOffsetを検査しているため、
	// Num32BitValues - Offsetで符号なし整数の
	// アンダーフローは発生しない。
	FWK_ASSERT_RETURN_IF(a_rootConstantCount > l_rootParameter.Constants.Num32BitValues - a_destinationOffset, "Root32BitConstantsの設定数がRootSignatureで確保した個数を超えています。");

	// SetComputeRoot32BitConstants(RootParameterIndex, 
	//								設定する32bit値の個数、
	//								設定元データの先頭アドレス、
	//								RootConstants内の書き込み開始位置);
	l_computeCommandList->SetComputeRoot32BitConstants(l_rootParameterIndex,
													   a_rootConstantCount,
												       a_rootConstantData,
													   a_destinationOffset);
}