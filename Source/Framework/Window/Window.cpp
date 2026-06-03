#include "Window.h"

// IMGUIのWind32用メッセージ処理関数
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

	// クライアント領域のサイズが設定値通りになるようにウィンドウサイズを調整する
	SetupClientSize();

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

const FWK::Struct::WindowResizeRequest& FWK::Window::ThrowResizeRequest()
{
	m_resizeRequest.m_isRequested = false;

	return m_resizeRequest;
}

bool FWK::Window::HasHWND() const
{
	return m_hwnd ? true : false;
}

void FWK::Window::SaveCONFIG() const
{
	const auto& l_rootJson = m_jsonConverter.Serialize(*this);

	Utility::SaveJsonFile(l_rootJson, k_configFileIOPath);
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
	
	// 戻り値として、このウィンドウを識別するためのHWNDを受け取る
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

void FWK::Window::SetupClientSize()
{
	// まだウィンドウが作成されていないなら何もしない
	if (!m_hwnd) { return; }

	// MoveWindow は、ウィンドウ全体の位置とサイズを変更する関数
	// この関数では、結果としてクライアント領域が目的サイズになるように調整して使う
	// MoveWindow
	// (
	//		対象のウィンドウハンドル、
	//		ウィンドウ左上のX座標、
	//		ウィンドウ左上のY座標、
	//		ウィンドウ全体の幅、
	//		ウィンドウ全体の高さ、
	//		再描画をするかどうか、
	// );
	if(m_windowStyleTag == Utility::GetVALTag<Tag::WindowStyleBorderlessFullScreenTag>())
	{
		// メイン画面の横幅、高さをピクセル単位で取得
		const int l_screenWidth  = GetSystemMetrics(SM_CXSCREEN);
		const int l_screenHeight = GetSystemMetrics(SM_CYSCREEN);

		MoveWindow(m_hwnd,
				   k_defaultWindowPositionX,
				   k_defaultWindowPositionY,
				   l_screenWidth,
				   l_screenHeight,
				   TRUE);

		// ボーダーレスフルウィンドウのためクライアント領域かどうかを計算する必要がない
		m_clientSize.m_width  = static_cast<std::uint32_t>(l_screenWidth);
		m_clientSize.m_height = static_cast<std::uint32_t>(l_screenHeight);

		return;
	}
	else if (m_windowStyleTag == Utility::GetVALTag<Tag::WindowStyleNormalTag>())
	{
		// 通常のウィンドウには枠やタイトルバーがあるため、
		//「描画中に使う中身の領域(クライアント領域)」と
		// 「ウィンドウ全体の大きさ」は一致しない
		RECT l_rcWND    = {};
		RECT l_rcClient = {};

		// ウィンドウ全体の大きさと、クライアント領域の大きさを取得する
		GetWindowRect(m_hwnd, &l_rcWND);
		GetClientRect(m_hwnd, &l_rcClient);

		// ウィンドウ全体からクライアント領域を引くことで、
		// 枠やタイトルバーが占めているサイズを求める
		const int l_frameWidth  = (l_rcWND.right  - l_rcWND.left) - (l_rcClient.right  - l_rcClient.left); 
		const int l_frameHeight = (l_rcWND.bottom - l_rcWND.top)  - (l_rcClient.bottom - l_rcClient.top);

		// 欲しいクライアント領域サイズに、枠やタイトルバー分を足したサイズで
		// ウィンドウ全体を作り直す
		MoveWindow(m_hwnd,
				   l_rcWND.left,
				   l_rcWND.top,  
				   static_cast<int>(m_clientSize.m_width)  + l_frameWidth, 
				   static_cast<int>(m_clientSize.m_height) + l_frameHeight,
				   TRUE);

		return;
	}

	FWK_ASSERT_RETURN("ウィンドウスタイルタグの取得に失敗しておりクライアントサイズの設定を行えませんでした。");
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
	m_resizeRequest.m_clientSize  = a_clientSize;
	m_resizeRequest.m_isRequested = true;;
	m_resizeRequest.m_isMinimized = a_wPARAM == SIZE_MINIMIZED;

	// 最小化中は、クライアント領域が0になることがあるためreturn
	if (m_resizeRequest.m_isMinimized) { return; }

	if (m_clientSize.m_width  == k_invalidClientWidth ||
		m_clientSize.m_height == k_invalidClientHeight) 
	{
		return;
	}

	m_clientSize = m_resizeRequest.m_clientSize;
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
	if	   (m_windowStyleTag == Utility::GetVALTag<Tag::WindowStyleBorderlessFullScreenTag>()) { return WS_POPUP; }
	else if(m_windowStyleTag == Utility::GetVALTag<Tag::WindowStyleNormalTag>())		       { return k_generalWindowStyle; }

	FWK_ASSERT_RETURN_VALUE("ウィンドウスタイルタグの取得に失敗しておりクライアントサイズの設定を行えませんでした。", k_generalWindowStyle);
}