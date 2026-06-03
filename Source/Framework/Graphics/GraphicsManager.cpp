#include "GraphicsManager.h"

void FWK::Graphics::GraphicsManager::INIT()
{
#if defined(_DEBUG)
	FWK_ASSERT_RETURN_IF_FAILED(!EnableDebugLayer(), "デバッグレイヤーの有効化に失敗しました。");
#endif
}


#if defined(_DEBUG)
bool FWK::Graphics::GraphicsManager::EnableDebugLayer() const
{
	TypeAlias::ComPtr<ID3D12Debug5> l_debug = nullptr;

	// デバッグ機能を有効化するためのインターフェースを取得する関数
	// D3D12GetDebugInterface(受け取りたいCOMインターフェース型のID、
	//					      作成結果のポインタを書き込むアドレス);
	auto l_hr = D3D12GetDebugInterface(IID_PPV_ARGS(l_debug.ReleaseAndGetAddressOf()));

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(FAILED(l_hr), "デバッグレイヤーの有効化に失敗しました。", false);

	l_debug->EnableDebugLayer();

	return true;
}
#endif