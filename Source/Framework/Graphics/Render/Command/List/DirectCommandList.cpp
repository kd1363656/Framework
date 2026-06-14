#include "DirectCommandList.h"

FWK::Graphics::DirectCommandList::DirectCommandList() : 
	CommandListBase(Constant::k_createCommandListTypeDirect)
{}
FWK::Graphics::DirectCommandList::~DirectCommandList() = default;

void FWK::Graphics::DirectCommandList::TransitionResourceBarrier(const TypeAlias::ComPtr<ID3D12Resource2>& a_resource, const D3D12_RESOURCE_STATES a_beforeState, const D3D12_RESOURCE_STATES a_afterState) const
{
	FWK_ASSERT_RETURN_IF_FAILED(!a_resource,	 "状態遷移予定のリソースが無効になっているため、リソースの遷移に失敗しました。");

	if (a_beforeState == a_afterState) { return; }

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

void FWK::Graphics::DirectCommandList::SetupRenderPipeline(const std::weak_ptr<PipelineState>& a_pipelineState) const
{
	const auto& l_directCommandList = GetREFCommandList();

	FWK_ASSERT_RETURN_IF_FAILED(!l_directCommandList, "ダイレクトコマンドリストが作成されておらず、パイプラインの設定に失敗しました。");

	const auto& l_pipelineState = a_pipelineState.lock();

	FWK_ASSERT_RETURN_IF_FAILED(!l_pipelineState, "パイプラインステートが無効なため、パイプラインの設定に失敗しました。");

	const auto& l_rootSignature = l_pipelineState->GetREFUseRootSignature().lock();

	FWK_ASSERT_RETURN_IF_FAILED(!l_rootSignature, "パイプラインステートで使用するルートシグネチャが無効なため、パイプラインの設定に失敗しました。");

	const auto& l_d3dRootSignature = l_rootSignature->GetREFRootSignature();

	FWK_ASSERT_RETURN_IF_FAILED(!l_d3dRootSignature, "ルートシグネチャが作成されておらず、パイプラインの設定に失敗しました。");

	const auto& l_d3dPipelineState = l_pipelineState->GetREFPipelineState();

	FWK_ASSERT_RETURN_IF_FAILED(!l_d3dPipelineState, "パイプラインステートが作成されておらず、パイプラインの設定に失敗しました。");
	
	// コマンドリストにルートシグネチャを設定する関数
	// SetGraphicsRootSignature(描画パイプラインで使用するルートシグネチャのポインタ);
	// ルートシグネチャは、シェーダーにどのリソースをどう渡すかを表す設定情報
	// これを先に設定しておかないと、後続の描画で使用するリソースの結び付けルールが決まらない
	l_directCommandList->SetGraphicsRootSignature(l_d3dRootSignature.Get());

	// コマンドリストにパイプラインステートをセットする関数
	// SetPipelineState(パイプラインステートのポインタ)
	// PSO(PipelineStateObject)には、
	// どのシェーダーを使うか、
	// どうラスタライズするか
	// 深度テストを使うか、など
	// 描画パイプラインの重要な設定がまとめて入っている
	l_directCommandList->SetPipelineState(l_d3dPipelineState.Get());
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

void FWK::Graphics::DirectCommandList::DispatchMesh(const UINT a_threadCountGroupX, const UINT a_threadCountGroupY, const UINT a_threadCountGroupZ) const
{
	const auto& l_directCommandList = GetREFCommandList();

	FWK_ASSERT_RETURN_IF_FAILED(!l_directCommandList, "ダイレクトコマンドリストが作成されておらず、DispatchMesh処理に失敗しました。");

	// メッシュシェーダーを実行するための関数
	// DispatchMesh(X方向のグループ数、
	//				Y方向のグループ数、
	//				Z方向のグループ数);
	l_directCommandList->DispatchMesh(a_threadCountGroupX, a_threadCountGroupY, a_threadCountGroupZ);
}