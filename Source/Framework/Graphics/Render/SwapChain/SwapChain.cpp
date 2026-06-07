#include "SwapChain.h"

void FWK::Graphics::SwapChain::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}
bool FWK::Graphics::SwapChain::Create(const Window&						  a_window, 
									  const Device&						  a_device,	
									  const Factory&					  a_factory,
									  const DirectCommandQueue&			  a_directCommandQueue,
											TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool)
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!CreateSwapChain(a_window, a_factory, a_directCommandQueue), "スワップチェインの作成に失敗しました。",     false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!CreateBackBufferList(a_device, a_rtvDescriptorPool),        "バックバッファリストの作成に失敗しました。", false);

	return true;
}
void FWK::Graphics::SwapChain::PostCreateSetup(const HWND& a_hwnd, const Factory& a_factory) const
{
	const auto& l_factory = a_factory.GetREFFactory();

	FWK_ASSERT_RETURN_IF_FAILED(!l_factory, "ファクトリーの作成に失敗しており、排他スクリーン設定が出来ませんでした。");

	// ウィンドウとDXGIの関連設定を行う関数
	// MakeWindowAssociation(対象のウィンドウハンドル、
	//						 ウィンドウに対して適用するDXGIの動作設定);
	// Alt + Enterで勝手に排他フルスクリーンに切り替わるのを防ぐ
	l_factory->MakeWindowAssociation(a_hwnd, DXGI_MWA_NO_ALT_ENTER);
}

void FWK::Graphics::SwapChain::Present() const
{
	// スワップチェインが存在しなければ画面表示できないのでreturn
	FWK_ASSERT_RETURN_IF_FAILED(!m_swapChain, "ファクトリーの作成に失敗しており、排他スクリーン設定ができませんでした。");

	// 現在のバックバッファを前面に出して画面へ表示する
	// Present(垂直同期の間隔、
	//		   追加の表示オプション)
	m_swapChain->Present(m_syncInterval, k_swapChainPresentFlagNone);
}

nlohmann::json FWK::Graphics::SwapChain::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

bool FWK::Graphics::SwapChain::Resize(const Device&					      a_device,
									  const ResourceReleaseContext&       a_resourceReleaseContext, 
									  const Struct::ClientSize&		      a_clientSize,
										    TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool)
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_swapChain,							"スワップチェインが作成されていないため、リサイズ処理に失敗しました。",     false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!IsValidBackBufferSize(a_clientSize), "リサイズ後のバックバッファサイズが無効です、リサイズ処理に失敗しました。", false);

	// ResizeBuffers()は、古いBackBufferへの参照が残っていると失敗する。
	// そのため、先にBackBufferのComPtrとRTV用DescriptorIndexを解放する
	for (auto& l_backBuffer : m_backBufferList)
	{
		l_backBuffer.m_rtvDescriptorIndex = a_resourceReleaseContext.ReleaseRenderTargetResourceImmediately(l_backBuffer.m_rtvDescriptorIndex, l_backBuffer.m_backBufferResource, a_rtvDescriptorPool);
	}
	
	// ResizeBufferは、SwapChain内部のBackBufferを新しい左図で作り直すDXGI関数
	// ResizeBuffers(バッファ枚数、
	//				 新しい横幅、
	//				 新しい縦幅、
	//				 バックバッファのフォーマット、
	//			     スワップチェイン作成時と同じ追加フラグ);
	const auto l_hr = m_swapChain->ResizeBuffers(static_cast<UINT>(m_backBufferList.size()),
												 a_clientSize.m_width,
												 a_clientSize.m_height,
												 Constant::k_defaultSwapChainBackBufferFormat,
											     k_swapChainDescFlags);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(FAILED(l_hr), "スワップチェインのResizeBufferに失敗しており、リサイズ処理に失敗しました。", false);

	// ResizeBuffers後は、SwapChain内部のBackBuffersが新しくなっている。
	// そのため、GetBufferで新しいBackBufferを取得し直し、RTVも作り直す。
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!CreateBackBufferList(a_device, a_rtvDescriptorPool), "リサイズ後のバックバッファリスト作成に失敗しており、リサイズ処理に失敗しました。", false);

	return true;
}

void FWK::Graphics::SwapChain::ResizeBackBufferList(const std::size_t a_backBufferNUM)
{
	m_backBufferList.resize(a_backBufferNUM);
}

UINT FWK::Graphics::SwapChain::FetchVALCurrentBackBufferIndex() const
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_swapChain, "スワップチェインが作成されておらず、現在のバックバッファのインデックスの取得に失敗しました。", k_invalidBackBufferNUM);

	return m_swapChain->GetCurrentBackBufferIndex();
}

bool FWK::Graphics::SwapChain::CreateSwapChain(const Window& a_window, const Factory& a_factory, const DirectCommandQueue& a_directCommandQueue)
{
	const auto& l_factory = a_factory.GetREFFactory();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_factory, "ファクトリーの作成がされておらず、スワップチェインの作成に失敗しました。", false);

	// スワップチェインは描画結果を表示するための仕組みなので、描画に使う
	// Directタイプのコマンドキューである必要がある
	// CopyやCompute用のコマンドキューでは画面表示用のスワップチェインを作ることはできないのでreturn
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_directCommandQueue.GetVALCreateCommandListType() != D3D12_COMMAND_LIST_TYPE_DIRECT, "コマンドキューのコマンドリストタイプは描画可能なDirectでないといけません、スワップチェインの作成に失敗しました。", false);
	
	const auto& l_commandQueue = a_directCommandQueue.GetREFCommandQueue();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_commandQueue, "コマンドキューの作成がされていないためスワップチェインが作成できません、スワップチェインの作成に失敗しました。", false);

	// バックバッファ枚数が0だと表示用バッファを一枚も持てないので作成できない。
	// 通常は2枚以上にすることが多い
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(m_backBufferList.empty(), "バックバッファリストの中身が空になっており、スワップチェインの作成に失敗しました。", false);
	
	// スワップチェインの作成設定をまとめた構造体
	// バッファサイズ、バッファ枚数、入れ替え方式などをここで指定する。
	DXGI_SWAP_CHAIN_DESC1 l_desc = {};

	const auto& l_clientSize = a_window.GetREFClientSize();

	// バックバッファ1枚当たりの横幅
	l_desc.Width = l_clientSize.m_width;

	// バックバッファ一枚当たりの縦幅
	l_desc.Height = l_clientSize.m_height;

	// バックバッファの画素フォーマット
	// 1ピクセルをどういう並びの色データとして持つかを表す。
	// R8G8B8A8_UNORMは、RGBA各8bitの標準的なカラー形式
	// ※注意 : RTVを作るときのフォーマットとも基本的にそろえる必要がある
	l_desc.Format = Constant::k_defaultSwapChainBackBufferFormat;

	// 立体視表示を使うかどうか
	// 通常の2D/3Dでは使わないためFALSEにする
	l_desc.Stereo = FALSE;

	// マルチサンプリング数
	// 1はMSAAを使わない通常設定
	// 4などにするとアンチエイリアス用途になるが、
	// スワップチェインのバックバッファでは1を使うことが多い
	l_desc.SampleDesc.Count = Constant::k_defaultSampleCount;

	// マルチサンプリング品質レベル
	// 通常設定では0を使うことが多い
	l_desc.SampleDesc.Quality = Constant::k_defaultSampleQuality;

	// このバッファを何に使うかを指定する
	// DXGI_USAGE_RENDER_TARGET_OUTPUTは
	// 「レンダーターゲットとして描画結果を書き込む用途」で使う設定
	l_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// バックバッファの枚数
	// 2ならダブルバッファ、3ならトリプルバッファになる。
	// 枚数を増やすとGPU / CPUとの並列性が上がる場合があるが、
	// その分メモリも使用する
	l_desc.BufferCount = static_cast<UINT>(m_backBufferList.size());

	// バックバッファサイズと表示サイズが違うときの拡大縮小方法
	//STRETCHは表示先に合わせて引き延ばす設定
	l_desc.Scaling = DXGI_SCALING_STRETCH;

	// バックバッファの入れ替え方式
	// FLIP_DISCARDはDirectX12で一般的なフリップモデルの設定
	// 昔のDISCARD \ SEQUENTIALより現代的な方法で、基本的にこれを使うことが多い
	l_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	// α合成の扱い。
	// 通常のウィンドウ描画ではUNSPECIFIEDで問題ないことが多い
	l_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	// 追加オプションフラグ
	// 今回は特別な機能を使わないので0
	// 例えばtearing対応などを入れる場合はここにフラグを追加する
	l_desc.Flags = k_swapChainDescFlags;

	TypeAlias::ComPtr<IDXGISwapChain1> l_swapChain = nullptr;

	// スワップチェイン作成関数
	// CreateSwapChainForHwnd(表示に使うコマンドキュー、
	//						  表示先ウィンドウハンドル、
	//						  スワップチェイン設定、
	//						  フルスクリーン詳細設定(nullptrなら未設定)、
	//						  出力先モニター制限(nullptrなら未指定)
	//						  作成したスワップチェインの受け取り先);
	auto l_hr = l_factory->CreateSwapChainForHwnd(l_commandQueue.Get(), 
												  a_window.GetREFHWND(),
												  &l_desc, 
												  nullptr,
												  nullptr,
												  l_swapChain.ReleaseAndGetAddressOf());

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(FAILED(l_hr), "スワップチェインの作成に失敗しました。", false);

	// As関数はCOMのQueryInterfaceを使って安全に型変換する。
	// ここではIDXGISwapChain1からメンバーが持つ方へ変換している
	l_hr = l_swapChain.As(&m_swapChain);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(FAILED(l_hr), "スワップチェインの型変換に失敗しました、スワップチェインの作成に失敗しました。", false);

	return true;
}

bool FWK::Graphics::SwapChain::CreateBackBufferList(const Device& a_device, TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool)
{
	const auto& l_device = a_device.GetREFDevice();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_device, "デバイス作成が行われておらず、バックバッファーの作成に失敗しました。", false);

	// バックバッファはスワップチェインが内部に持っており、
	// 先にスワップチェインが作成されていないと取得できないのでreturn
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_swapChain, "スワップチェインが作成されておらず、バックバッファーの作成に失敗しました。", false);

	// リソースを「レンダーターゲットとしてどう見るか」を指定する構造体
	// RTVは「このリソースを描画先として扱うためのビュー情報」だと思えばよい
	D3D12_RENDER_TARGET_VIEW_DESC l_rtvDesc = {};

	// RTVから見たときの画素フォーマット
	// ※注意 : バックバッファの実際のフォーマットと基本的にそろえる必要がある
	l_rtvDesc.Format = Constant::k_defaultSwapChainBackBufferFormat;

	// このRTVが「2Dテクスチャ」としてバックバッファを見ることを指定する。
	// スワップチェインのバックバッファは通常2Dテクスチャとして扱う
	l_rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	// バックバッファーを一枚ずつ取得して、それぞれに対応するRTVを作成する
	for (auto l_backBufferIndex = 0U; l_backBufferIndex < static_cast<UINT>(m_backBufferList.size()); ++l_backBufferIndex)
	{
		// スワップチェインが内部に持っているバックバッファリソースを取得する関数
		// GetBuffer(取得したいバックバッファーのインデックス、
		//			 受け取りたいCOMインターフェース型のID、
		//			 作成結果のポインタを書き込むアドレス);
		const auto l_hr = m_swapChain->GetBuffer(l_backBufferIndex, IID_PPV_ARGS(m_backBufferList[l_backBufferIndex].m_backBufferResource.ReleaseAndGetAddressOf()));

		FWK_ASSERT_RETURN_VALUE_IF_FAILED(FAILED(l_hr), "スワップチェインとバックバッファーの紐づけに失敗ており、バックバッファーの作成に失敗しました。。", false);

		// レンダーターゲット用アロケータを進める
		const auto l_rtvDescriptorIndex = a_rtvDescriptorPool.Allocate();

		FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_rtvDescriptorIndex == Constant::k_invalidDescriptorIndex, "バックバッファ用RTVIndexの確保に失敗しており、バッファーの作成に失敗しました。", false);
		
		// l_backBufferIndex番目のバックバッファと紐づいているRTVのStorageIDを格納
		m_backBufferList[l_backBufferIndex].m_rtvDescriptorIndex = l_rtvDescriptorIndex;

		// RTVを置くディスクリプタ位置を取得する
		// l_i番目のバックバッファに対応するRTVも、同じl_i番目の位置に書いている
		const auto& l_rtvHandle = a_rtvDescriptorPool.FetchVALCPUDescriptorHandle(l_rtvDescriptorIndex);

		// このバックバッファを「描画先」として使えるようにする
		// レンダーターゲットビューを作成する関数
		// CreateRenderTargetView(RTVを作りたい対象リソース、
		//						  RTVの見え方を指定する設定、
		//						  作成したRTVを書き込むディスクリプタ位置);
		l_device->CreateRenderTargetView(m_backBufferList[l_backBufferIndex].m_backBufferResource.Get(), &l_rtvDesc, l_rtvHandle);
	}

	
	return true;
}

bool FWK::Graphics::SwapChain::IsValidBackBufferSize(const Struct::ClientSize& a_clientSize) const
{
	if (a_clientSize.m_width  == Constant::k_invalidClientWidth ||
		a_clientSize.m_height == Constant::k_invalidClientHeight)
	{
		return false;
	}

	return true;
}