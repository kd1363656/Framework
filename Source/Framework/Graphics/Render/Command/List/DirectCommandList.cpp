#include "DirectCommandList.h"

FWK::Graphics::DirectCommandList::DirectCommandList() : 
	CommandListBase(Constant::k_createCommandListTypeDirect)
{}
FWK::Graphics::DirectCommandList::~DirectCommandList() = default;

void FWK::Graphics::DirectCommandList::TransitionResourceBarrier(const TypeAlias::ComPtr<ID3D12Resource2>& a_resource, const D3D12_RESOURCE_STATES a_beforeState, const D3D12_RESOURCE_STATES a_afterState) const
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
	l_directCommandList->ResourceBarrier(k_singleSetupBarrierNUM, &l_barrier);
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

void FWK::Graphics::DirectCommandList::SetupRenderTarget(const TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool, const UINT a_rtvDescriptorIndex) const
{
	FWK_ASSERT_RETURN_IF_FAILED(a_rtvDescriptorIndex == Constant::k_invalidDescriptorIndex, "デスクリプタヒープインデックスが無効な値のため、レンダーターゲットの設定し失敗しました。");
	
	const auto& l_directCommandList = GetREFCommandList();

	FWK_ASSERT_RETURN_IF_FAILED(!l_directCommandList, "ダイレクトコマンドリストが作成されておらず、レンダーターゲットの設定に失敗しました。");

	const auto& l_handle = a_rtvDescriptorPool.FetchVALCPUDescriptorHandle(a_rtvDescriptorIndex);

	// OMステージにレンダーターゲットを設定する関数
	// OMSetRenderTargets(設定するレンダーターゲット数、
	//					  レンダーターディスクリプタ配列の先頭アドレス、
	//					  ディスクリプタが連続は位置かどうか、
	//					　深度ステンシルビューのアドレス);
	l_directCommandList->OMSetRenderTargets(k_singleSetupRenderTargetNUM,
											&l_handle,
										    FALSE,
											nullptr);
}
void FWK::Graphics::DirectCommandList::ClearRenderTarget(const TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool, const UINT a_rtvDescriptorIndex, const TypeAlias::Math::Color& a_clearColor) const
{
	FWK_ASSERT_RETURN_IF_FAILED(a_rtvDescriptorIndex == Constant::k_invalidDescriptorIndex, "デスクリプタヒープインデックスが無効な値のため、レンダーターゲットのクリアに失敗しました。");

	const auto& l_directCommandList = GetREFCommandList();

	FWK_ASSERT_RETURN_IF_FAILED(!l_directCommandList, "ダイレクトコマンドリストが作成されておらず、レンダーターゲットのクリアに失敗しました。");

	const auto& l_handle = a_rtvDescriptorPool.FetchVALCPUDescriptorHandle(a_rtvDescriptorIndex);

	// 現在のレンダーターゲットを指定色でクリアする関数
	// ClearRenderTargetView(クリア対象のRTVハンドル、
	//						 クリア色RGBA配列、
	//						 部分クリアする矩形数(0の場合は矩形指定なしとみなし全面クリア)、
	//						 矩形配列の先頭アドレス);
	l_directCommandList->ClearRenderTargetView(l_handle,
											   &a_clearColor.x,
											   k_allRECTClear,
											   nullptr);
}

void FWK::Graphics::DirectCommandList::SetupRenderArea(const RenderArea& a_renderArea) const
{
	const auto& l_directCommandList = GetREFCommandList();

	FWK_ASSERT_RETURN_IF_FAILED(!l_directCommandList, "ダイレクトコマンドリストが作成されておらず、ビューポートとシザー矩形の設定が出来ませんでした。");

	// Viewportは頂点変換後の座標(NDC -1.0F ~ 1.0F)を、実際の画面上のどこに描くかを決める設定
	// Viewportをコマンドリストへ設定する関数
	// RSSetViewports(設定するViewportの数、
	//				  Viewport構造体のアドレス);
	l_directCommandList->RSSetViewports(k_setViewportNUM, &a_renderArea.GetREFViewport());

	// ScissorRectをコマンドリストへ設定する
	// ScissorRectは実際に描画してよいピクセル範囲を制限する四角形
	// RSSetScissorRects(設定するScissorRectの数、
	//					 ScissorRectの先頭アドレス)
	l_directCommandList->RSSetScissorRects(k_setScissorRectNUM, &a_renderArea.GetREFScissorRECT());
}

void FWK::Graphics::DirectCommandList::SetupConstantBufferView(const D3D12_GPU_VIRTUAL_ADDRESS& a_gpuVirtualAddress, const RootSignature& a_rootSignature, const Enum::RootParameterType a_rootParameterType) const
{
	const auto& l_directCommandList = GetREFCommandList();

	FWK_ASSERT_RETURN_IF_FAILED(!l_directCommandList, "ルートシグネチャが作成されておらず、定数バッファビュー設定に失敗しました。");

	const auto l_rootParameterIndex = a_rootSignature.FindVALRootParameterIndex(a_rootParameterType);

	FWK_ASSERT_RETURN_IF_FAILED(l_rootParameterIndex == Constant::k_invalidRootParameterIndex, "パラメータインデックスが無効なため、定数バッファビュー設定に失敗しました。");

	// RootSignature側でD3D12_ROOT_PARAMETER_TYPE_CBVにした場所へ、
	// UploadBuffer上の定数バッファ位置を直接結びつける
	// SetGraphicsRootConstantBufferView(ルートパラメータ番号、
	//									 CBVとして参照させるGPU仮想アドレス);
	l_directCommandList->SetGraphicsRootConstantBufferView(l_rootParameterIndex, a_gpuVirtualAddress);
}

void FWK::Graphics::DirectCommandList::AddTransitionResourceBarrier(const TypeAlias::ComPtr<ID3D12Resource2>& a_resource, const D3D12_RESOURCE_STATES& a_beforeState, const D3D12_RESOURCE_STATES& a_afterState)
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
