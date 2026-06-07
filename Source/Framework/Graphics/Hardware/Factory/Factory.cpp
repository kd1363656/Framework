#include "Factory.h"

bool FWK::Graphics::Factory::Create()
{
	// DXGIファクトリー作成時に渡すオプションフラグ
	// 通常は0でよいが、デバッグ時はデバッグ用フラグを追加する
	auto l_flags = k_defaultFactoryCreateFlags;

#if defined(_DEBUG)
	// Debugビルド時はDXGI_CREATE_FACTORY_DEBUGを有効にする
	// これにより、DXGI関連の問題をデバッグレイヤーが検出しやすくなる
	l_flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	// DXGIファクトリーを作成する関数
	// CreateDXGIFactory2(ファクトリー作成時のオプションを指定する、
	//					  受け取りたいCOMインターフェース型のID、
	//					  作成結果のポインタを書き込むアドレス);
	auto l_hr = CreateDXGIFactory2(l_flags, IID_PPV_ARGS(m_factory.ReleaseAndGetAddressOf()));

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(FAILED(l_hr), "ファクトリーの作成に失敗しました。", false);

	return true;
}