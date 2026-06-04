#include "Window.h"

// IMGUIのWin32用メッセージ処理関数
// ImGui_ImplWin32_WndProcHandler(ウィンドウハンドル、
//								  Windowsメッセージ、
//								  WPARAM,
//								  LPARAM);
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND,
															 UINT,
															 WPARAM,
															 LPARAM);

FWK::Window::Window() : 
	m_hwnd(nullptr),

	m_normalWindowRECT(),

	m_clientSize   (),
	m_resizeRequest(),

	m_windowStyleTag(Utility::GetVALTag<Tag::WindowStyleNormalTag>())

{}
FWK::Window::~Window()
{
	// 必ず開放処理が呼ばれるようにする
	Release();
}

void FWK::Window::LoadCONFIG()
{
	const auto& l_rootJson = Utility::LoadJsonFile(k_configFileIOPath);

	if (l_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(l_rootJson, *this);
}
void FWK::Window::PostLoadCONFIG(const std::wstring& a_windowClassName, const std::string& a_titleName)
{
	FWK_ASSERT_RETURN_IF_FAILED(!CreateWindowInstance(a_windowClassName, a_titleName), "ウィンドウインスタンスの作成に失敗しました。");

	// CONFIGから読み込んだ表示形態を、実際のウィンドウに反映する
	ApplyWindowStyle();

	// ウィンドウの表示
	ShowWindow(m_hwnd, SW_SHOW);

	// ウィンドウのクライアント領域をすぐに再描画
	UpdateWindow(m_hwnd);

	// timeGetTime関数の精度を1msに設定(Sleep関数などに影響する)
	timeBeginPeriod(k_timeResolutionMS);
}

bool FWK::Window::ProcessMessages() const
{
	// Windowsから届いているメッセージを受け取るための変数
	MSG l_msg = {};

	// メッセージキューにたまっているメッセージを順番に取り出して処理する関数
	// PeekMessage(取り出したメッセージの書き込み先、
	//			　 nullptrを渡すと、このスレッドの全ウィンドウを対象にする、
	//			　 取得するメッセージ番号の下限、
	//			   取得するメッセージ番号の上限、
	//			   取り出したメッセージをどうするか);
	while (PeekMessage(&l_msg,
					   nullptr,
					   k_msgFilterMIN,
					   k_msgFilterMAX,
					   PM_REMOVE))
	{
		// 終了メッセージが来たら、呼びだし元に終了を知らせる
		if (l_msg.message == WM_QUIT) { return false; }

		// キー入力メッセージを、文字入力メッセージに変換する
		// 例 : キーボードのAキー入力から'A'文字の入力メッセージを作る
		TranslateMessage(&l_msg);

		// 取り出したメッセージを対応するウィンドウプロシージャへ送る
		DispatchMessage(&l_msg);
	}

	// 終了メッセージが来ていないので、処理を続けてよい
	return true;
}

void FWK::Window::ClearResizeRequest()
{
	// ウィンドウのサイズ変更は一回だけ通知する
	m_resizeRequest.m_isRequested = false;
}

void FWK::Window::SaveCONFIG() const
{
	const auto& l_rootJson = m_jsonConverter.Serialize(*this);

	Utility::SaveJsonFile(l_rootJson, k_configFileIOPath);
}

bool FWK::Window::IsMinimized() const
{
	return m_resizeRequest.m_isMinimized;
}

void FWK::Window::SetupWindowStyleTag(const TypeAlias::TagType a_windowStyleTag)
{
	if (m_windowStyleTag == a_windowStyleTag) { return; }

	// 現在のウィンドウが通常ウィンドウならウィンドウ形式を切り替える前に
	// 元の位置とサイズを保存しておく
	if (m_windowStyleTag == Utility::GetVALTag<Tag::WindowStyleNormalTag>())
	{
		StoreNormalWindowRECT();
	}

	m_windowStyleTag = a_windowStyleTag;

	// CONFIG読み込み中など、まだウィンドウが作られていない場合はここで終了
	if (!m_hwnd) { return; }

	ApplyWindowStyle();
}

LRESULT FWK::Window::CallWindowProcedure(const HWND   a_hwnd, 
										 const UINT   a_message, 
									     const WPARAM a_wPARAM,
										 const LPARAM a_lPARAM)
{
	// ウィンドウに関連付けたWindowクラスのインスタンスアドレスを取得する
	auto* l_this = static_cast<Window*>(GetProp(a_hwnd, k_windowInstancePropertyName.data()));
	
	// まだインスタンスアドレスが登録されていない場合
	if (!l_this)
	{
		if (a_message == WM_CREATE)
		{
			// CreateWindowで渡した生成情報を取得する
			auto* l_createStruct = reinterpret_cast<CREATESTRUCT*>(a_lPARAM);

			// lpCreateParamsに渡しておいたWindowクラスのインスタンスアドレスを取り出す
			auto* l_window = static_cast<FWK::Window*>(l_createStruct->lpCreateParams);

			// このウィンドウに対応するWindowクラスのインスタンスアドレスを登録する
			// これにより次回以降は、このウィンドウから自分のクラスを見つけられる
			SetProp(a_hwnd, k_windowInstancePropertyName.data(), l_window);

			// WM_CREATEの処理は完了
			return k_wmCreateHandledResult;
		}
		else
		{
			// まだ自分のクラスへ処理を渡せていないので、Windowsの標準の処理に任せる
			return DefWindowProc(a_hwnd,
								 a_message,
								 a_wPARAM,
								 a_lPARAM);
		}
	}
	
	// 登録済みのウィンドウクラスのインスタンスへ処理を渡す
	return l_this->WindowProcedure(a_hwnd,
								   a_message,
								   a_wPARAM,
								   a_lPARAM);
}
LRESULT FWK::Window::WindowProcedure(const HWND   a_hwnd, 
									 const UINT   a_message, 
								     const WPARAM a_wPARAM, 
									 const LPARAM a_lPARAM)
{
	if (ImGui::GetCurrentContext() && ImGui_ImplWin32_WndProcHandler(a_hwnd, 
																	 a_message,
																	 a_wPARAM,
																	 a_lPARAM))
	{
		return k_windowProcedureHandledResult;
	}

	// Windowsから送られてきたメッセージの種類ごとに処理を分ける
	switch(a_message)
	{
		// ユーザーがウィンドウ右上の×ボタンを押したときに
		// 自分で作成したウィンドウ開放処理を実行
		case WM_CLOSE:
		{
			Release();
		}
		break;

		// ウィンドウが実際に破棄される直前に届くメッセージ
		case WM_DESTROY:
		{
			// このウィンドウに関連付けていたWindowクラスのインスタンス情報を削除する
			RemoveProp(a_hwnd, k_windowInstancePropertyName.data());

			// アプリ終了メッセージを送って、メッセージループを終了できるようにする
			PostQuitMessage(k_quitExitCode);
		}
		break;

		// ウィンドウのクライアント領域サイズが変更されたときに届くメッセージ
		case WM_SIZE:
		{
			// WM_SIZEのLPARAMには、クライアント領域の横幅と縦幅がまとめて入っている。
			// LOWORDで下位側の値、横幅を取り出し、HIWORDで上位側の値、縦幅を取り出す。
			const Struct::ClientSize l_changedClientSize = { static_cast<UINT>(LOWORD(a_lPARAM)) , static_cast<UINT>(HIWORD(a_lPARAM)) };

			ApplyClientSizeFromWMSize(l_changedClientSize, a_wPARAM);
		}
		break;

		default:
		{
			// 自分で処理しないメッセージはWindows標準の処理に任せる
			return DefWindowProc(a_hwnd,
								 a_message,
								 a_wPARAM,
								 a_lPARAM);
		}
		break;
	}

	// このメッセージは自分で処理済みであることを返す
	return k_windowProcedureHandledResult;
}

bool FWK::Window::CreateWindowInstance(const std::wstring& a_windowClassName, const std::string& a_titleName)
{
	// ハンドル(.exe)を取得
	const HINSTANCE& l_hInstance = FetchVALInstanceHandle();
	
	// マルチバイト文字列に変換
	const auto& l_titleName = Utility::StringToWideString(a_titleName);

	// ウィンドウクラス登録用の設定パラメータ
	WNDCLASSEX l_wc = {};

	// WNDCLASSEX構造体のサイズをWindowsに伝える
	l_wc.cbSize = sizeof(WNDCLASSEX);

	// ウィンドウの基本的な動作を設定する
	// 例 : サイズ変更時に再描画するかどうかなど
	l_wc.style = k_windowStyle;

	// ウィンドウに届いたメッセージを処理する関数を登録する
	// ここでキーボード入力や閉じる操作などを受け取る
	l_wc.lpfnWndProc = &CallWindowProcedure;

	// ウィンドウクラス全体に追加で確保するメモリサイズ(未使用なら0)
	l_wc.cbClsExtra = k_classExtraBytes;

	// 各ウィンドウごとに追加で確保するメモリサイズ(未使用なら0)
	l_wc.cbWndExtra = k_windowExtraBytes;

	// このウィンドウクラスを所有するアプリ自身のハンドル
	l_wc.hInstance = l_hInstance;

	// 大きいアイコンを設定する
	l_wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);

	// 小さいアイコンを設定する
	l_wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

	// マウスカーソルの見た目を設定する
	l_wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	// 背景を塗るときに使うブラシを設定する
	l_wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

	// メニュー名を設定する。使わないのでnullptr
	l_wc.lpszMenuName = nullptr;

	// ウィンドウクラスを識別するための名前
	l_wc.lpszClassName = a_windowClassName.c_str();

	// ここまで設定したウィンドウクラス情報をWindowsに登録する
	// そうすることで、このクラス名を使ってウィンドウを作成できるようになる
	if (!RegisterClassEx(&l_wc)) { return false; }

	// 現在の設定から、実際に使うウィンドウスタイルを取得
	const DWORD l_style = FetchVALWindowStyle();

	// 登録したウィンドウクラスをもとに、実際のウィンドウを作成する関数
	// 戻り値として、このウィンドウを識別するためのHWNDを受け取る
	// CreateWindow(登録済みのウィンドウクラス名、
	//				タイトルバーに表示する文字列、
	//				ウィンドウの見た目や動作を決めるスタイル、
	//				ウィンドウ左上のX座標、
	//				ウィンドウ左上のY座標、
	//				ウィンドウの幅、
	//				ウィンドウの高さ、
	//				親ウィンドウがあるかどうか、
	//				メニューがあるかどうか、
	//				このアプリ自身のインスタンスハンドル、
	//				Windowクラス自身のアドレス。WM_CREATEで取り出してこのウィンドウに関連付ける); 
	m_hwnd = CreateWindow(a_windowClassName.c_str(),
						  l_titleName.c_str(),
						  l_style,
						  k_defaultWindowPositionX,
						  k_defaultWindowPositionY,
						  m_clientSize.m_width,
						  m_clientSize.m_height,
						  nullptr,
						  nullptr,
						  l_hInstance,
						  this);

	// ウィンドウ作成に失敗した場合、登録だけ成功して残るのを防ぐため
	// 先に登録したウィンドウクラスを解除する
	if (!m_hwnd)
	{
		UnregisterClass(a_windowClassName.c_str(), l_hInstance);
		return false;
	}

	return true;
}

void FWK::Window::SetupNormalWindowClientSize()
{
	// まだウィンドウが作成されていないなら何もしない
	if (!m_hwnd) { return; }

	if (m_windowStyleTag != Utility::GetVALTag<Tag::WindowStyleNormalTag>()) { return; }

	RECT l_clientRECT =
	{
		k_clientRECTLeft,
		k_clientRECTTop,
		static_cast<LONG>(m_clientSize.m_width),
		static_cast<LONG>(m_clientSize.m_height)
	};

	// 通常ウィンドウはタイトルバーや枠があるため、
	// 欲しいクライアント領域からウィンドウ全体サイズを逆算する
	FWK_ASSERT_RETURN_IF_FAILED(!AdjustWindowRect(&l_clientRECT, k_generalWindowStyle, FALSE), "通常ウィンドウのサイズ調整に失敗しました。");

	const auto l_windowWidth  = static_cast<int>(l_clientRECT.right  - l_clientRECT.left);
	const auto l_windowHeight = static_cast<int>(l_clientRECT.bottom - l_clientRECT.top);

	SetWindowPos(m_hwnd,
				 nullptr,
				 k_defaultWindowPositionX,
				 k_defaultWindowPositionY,
				 l_windowWidth,
				 l_windowHeight,
				 SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
}

void FWK::Window::Release()
{
	// まだウィンドウが作成されていないなら、解放するものがないので終了
	if (!m_hwnd) { return; }

	// Windowsに、このウィンドウを閉じて破棄してもらう
	DestroyWindow(m_hwnd);

	// もうこのウィンドウは使えない状態になったことを示すためnullptrを格納
	// ウィンドウの存在確認やゲームループの終了判定に使いやすくなる
	m_hwnd = nullptr;
}

void FWK::Window::ApplyClientSizeFromWMSize(const Struct::ClientSize& a_clientSize, const WPARAM& a_wPARAM)
{
	const bool l_isMinimized = a_wPARAM == SIZE_MINIMIZED;

	m_resizeRequest.m_isRequested = true;
	m_resizeRequest.m_isMinimized = l_isMinimized;
	
	if (l_isMinimized)
	{
		// 最小化中はクライアント領域が0x0になることがある、DX12で0x0のバックバッファ
		// にしてはいけないため、前回有効なm_clientSizeを保持する
		m_resizeRequest.m_clientSize = m_clientSize;

		return;
	}

	if (a_clientSize.m_width  == k_invalidClientWidth ||
		a_clientSize.m_height == k_invalidClientHeight) 
	{
		return;
	}

	m_clientSize				 = a_clientSize;
	m_resizeRequest.m_clientSize = m_clientSize;
}

void FWK::Window::ApplyWindowStyle()
{
	if (!m_hwnd) { return; }

	if (m_windowStyleTag == Utility::GetVALTag<Tag::WindowStyleNormalTag>())
	{
		ApplyNormalWindowStyle();
		return;
	}

	if (m_windowStyleTag == Utility::GetVALTag<Tag::WindowStyleBorderlessFullScreenTag>())
	{
		ApplyBorderlessFullScreenWindowStyle();
		return;
	}

	FWK_ASSERT_RETURN("未対応のウィンドウスタイルタグで、ウィンドウスタイルの適応に失敗しました。");
}

void FWK::Window::ApplyNormalWindowStyle()
{
	if (!m_hwnd) { return; }

	// ウィンドウの見た目を通常ウィンドウへ戻す
	SetWindowLongPtr(m_hwnd, GWL_STYLE, k_generalWindowStyle);

	const auto l_saveWindowWidth  = static_cast<int>(m_normalWindowRECT.right  - m_normalWindowRECT.left);
	const auto l_saveWindowHeight = static_cast<int>(m_normalWindowRECT.bottom - m_normalWindowRECT.top);

	if (const bool l_hasSavedWindowRECT = l_saveWindowWidth  > k_invalidClientWidth &&
										  l_saveWindowHeight > k_invalidClientWidth;
		l_hasSavedWindowRECT)
	{
		// ボーダーレスフルスクリーンへ移る前の通常ウィンドウ位置へ戻す
		SetWindowPos(m_hwnd,
					 nullptr,
					 static_cast<int>(m_normalWindowRECT.left),
					 static_cast<int>(m_normalWindowRECT.top),
					 l_saveWindowWidth,
					 l_saveWindowHeight,
					 SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
	}
	else
	{
		// 保存されたウィンドウの位置がない場合は、
		// 現在のクライアントサイズを使って通常ウィンドウサイズを作る
		SetupNormalWindowClientSize();
	}

	// SetWindowPos後の実際のクライアント領域を取得
	m_clientSize = FetchVALCurrentClientSize();

	RequestResizeFromClientSize(m_clientSize);
}

void FWK::Window::ApplyBorderlessFullScreenWindowStyle()
{
	if (!m_hwnd) { return; }

	// 通常ウィンドウ状態の位置とサイズを保存しておく
	StoreNormalWindowRECT();

	// 枠なしウィンドウへ変更する
	SetWindowLongPtr(m_hwnd, GWL_STYLE, k_borderlessFullScreenWindowStyle);

	// 現在のウィンドウに一番近いモニターを取得する
	const HMONITOR l_monitor = MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTONEAREST);

	MONITORINFO l_monitorINFO = {};
	
	l_monitorINFO.cbSize = sizeof(MONITORINFO);

	FWK_ASSERT_RETURN_IF_FAILED(!GetMonitorInfo(l_monitor, &l_monitorINFO), "モニター情報の取得に失敗しました。");

	const RECT& l_monitorRECT = l_monitorINFO.rcMonitor;

	const auto l_monitorWidth  = static_cast<int>(l_monitorRECT.right  - l_monitorRECT.left);
	const auto l_monitorHeight = static_cast<int>(l_monitorRECT.bottom - l_monitorRECT.top);

	// モニター全体を覆う位置とサイズに変更する
	SetWindowPos(m_hwnd,
				 HWND_TOP,
				 l_monitorRECT.left,
		         l_monitorRECT.top,
				 l_monitorWidth,
				 l_monitorHeight,
				 SWP_FRAMECHANGED | SWP_SHOWWINDOW);

	m_clientSize.m_width  = static_cast<UINT>(l_monitorWidth);
	m_clientSize.m_height = static_cast<UINT>(l_monitorHeight);

	RequestResizeFromClientSize(m_clientSize);
}

void FWK::Window::StoreNormalWindowRECT()
{
	if (!m_hwnd) { return; }

	// すでにボーダーレスフルスクリーン状態なら保存しない。
	if (m_windowStyleTag != Utility::GetVALTag<Tag::WindowStyleNormalTag>()) { return; }

	RECT l_windowRECT = {};

	if (!GetWindowRect(m_hwnd, &l_windowRECT)) { return; }

	m_normalWindowRECT = l_windowRECT;
}

void FWK::Window::RequestResizeFromClientSize(const Struct::ClientSize& a_clientSize)
{
	if (a_clientSize.m_width  == k_invalidClientWidth ||
		a_clientSize.m_height == k_invalidClientHeight)
	{
		return;
	}

	m_resizeRequest.m_clientSize  = a_clientSize;
	m_resizeRequest.m_isRequested = true;
	m_resizeRequest.m_isMinimized = false;
}

HINSTANCE FWK::Window::FetchVALInstanceHandle() const
{
	// nullptrを渡すと「今このプログラム自身(.exe)のモジュールハンドル」を取得する
	// ウィンドウクラス登録やウィンドウ作成時に必要になるため取得して返す
	return GetModuleHandle(nullptr);
}

DWORD FWK::Window::FetchVALWindowStyle() const
{
	// 持っているタグから返すウィンドウスタイルを判定する
	if      (m_windowStyleTag == Utility::GetVALTag<Tag::WindowStyleNormalTag>())		        { return k_generalWindowStyle; }
	else if	(m_windowStyleTag == Utility::GetVALTag<Tag::WindowStyleBorderlessFullScreenTag>()) { return k_borderlessFullScreenWindowStyle; }

	FWK_ASSERT_RETURN_VALUE("ウィンドウスタイルタグの取得に失敗しました。", k_generalWindowStyle);
}

FWK::Struct::ClientSize FWK::Window::FetchVALCurrentClientSize() const
{
	if (!m_hwnd) { return m_clientSize; }

	RECT l_clientRECT = {};

	if (!GetClientRect(m_hwnd, &l_clientRECT)) { return m_clientSize; }

	const Struct::ClientSize l_clientSize =
	{
		static_cast<UINT>(l_clientRECT.right  - l_clientRECT.left),
		static_cast<UINT>(l_clientRECT.bottom - l_clientRECT.top)
	};

	return l_clientSize;
}