#include "DirectCommandList.h"

FWK::Graphics::DirectCommandList::DirectCommandList() : 
	CommandListBase(Constant::k_createCommandListTypeDirect)
{}
FWK::Graphics::DirectCommandList::~DirectCommandList() = default;

void FWK::Graphics::DirectCommandList::TransitionResource(const TypeAlias::ComPtr<ID3D12Resource>& a_resource, const D3D12_RESOURCE_STATES a_beforeState, const D3D12_RESOURCE_STATES a_afterState) const
{
	FWK_ASSERT_RETURN_IF_FAILED(a_beforeState == a_afterState, "リソースの状態遷移前と後の遷移状態が全く一緒です、リソースの遷移に失敗しました。");
	FWK_ASSERT_RETURN_IF_FAILED(!a_resource,				   "状態遷移予定のリソースが無効になっているため、リソースの遷移に失敗しました。");

	const auto& l_directCommandList = GetREFCommandList();

	FWK_ASSERT_RETURN_IF_FAILED(!l_directCommandList, "ダイレクトコマンドリストが作成されておらず、リソースの遷移に失敗しました。");

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
	l_directCommandList->ResourceBarrier(k_sendBarrierNUM, &l_barrier);
}

void FWK::Graphics::DirectCommandList::AddTransitionResourceBarrier(const TypeAlias::ComPtr<ID3D12Resource>& a_resource, const D3D12_RESOURCE_STATES& a_beforeState, const D3D12_RESOURCE_STATES& a_afterState)
{
	FWK_ASSERT_RETURN_IF_FAILED(a_beforeState == a_afterState, "リソースの状態遷移前と後の遷移状態が全く一緒です、リソース一括遷移リストの要素としての追加に失敗しました。");
	FWK_ASSERT_RETURN_IF_FAILED(!a_resource,				   "状態遷移予定のリソースが無効になっているため、リソース一括遷移リストの要素としての追加に失敗しました。");

	// D3D12_RESOURCE_BARRIER構造体についての説明(CD3DX12_RESOURCE_BARRIER::Transition内部で使用)
	// Type                   : このバリアがどういうバリアであるかを指定
	// Flags                  : バリアの特別な追加設定
	// Transition.pResource   : 状態を切り替える対象のGPUリソース
	// Transition.StateBefore : 切り替える前のリソース状態
	// Transition.StateAfter  : 切り替えた後のリソース状態
	// Transition.Subresource : どのサブリソースを遷移対象にするか
	const auto& l_barrier = CD3DX12_RESOURCE_BARRIER::Transition(a_resource.Get(), a_beforeState, a_afterState);

	m_resourceBarrierTransitionBatchList.emplace_back(l_barrier);
}

void FWK::Graphics::DirectCommandList::FlushResourceBarrierTransitionBatch()
{
	const auto& l_directCommandList = GetREFCommandList();

	FWK_ASSERT_RETURN_IF_FAILED(!l_directCommandList, "ダイレクトコマンドリストが作成されておらず、リソースの一括遷移処理に失敗しました。");

	if (m_resourceBarrierTransitionBatchList.empty()) { return; }

	// リソースバリアを転送
	// ResourceBarrier(送るバリア数、
	//				   バリア情報の先頭アドレス)
	l_directCommandList->ResourceBarrier(static_cast<UINT>(m_resourceBarrierTransitionBatchList.size()), m_resourceBarrierTransitionBatchList.data());

	// リソース遷移が終われば次のフレームでも
	// 遷移に使う要素をキャッシュしているわけではないのでクリア
	m_resourceBarrierTransitionBatchList.clear();
}