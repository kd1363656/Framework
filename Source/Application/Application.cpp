#include "Application.h"

// SALアノテーション付きWinMain関数(各引数の意味や使用条件をコンパイラや静的解析ツールに伝えるための注釈)
// int WINAPI WinMain(この実行ファイル自体を表すインスタンスハンドル、
//					  昔のWindowsとの互換性のために渡される値、
//					  コマンドライン引数、
//					  ウィンドウを最初にどう表示するかを表す値)
int WINAPI WinMain(_In_     HINSTANCE,
	               _In_opt_ HINSTANCE,
	               _In_     LPSTR,
	               _In_     int)
{
	// デバッグ時にメモリリークを検出する設定
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// COMライブラリの初期化(WICやDXGI内部でも使用される)
	if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED))) { return Application::GetVALExitCodeCOMInitializeFailed(); }

	Application::GetInstance().Execute();

	// COM解放
	CoUninitialize();

	return Application::GetVALExitCodeSuccess();
}

void Application::Execute()
{
	LoadCONFIG();

	// ロードし終わった後の処理を実行(ウィンドウ生成など)
	PostLoadCONFIG();

	while (true)
	{
		// ウィンドウメッセージやアプリケーションを終了するかの処理をしているので
		// falseが戻り値ならbreakする
		if (!BeginFrame()) { break; }

		// リサイズ要求があればここで処理を行う
		ProcessResizeRequest();

		// 最小化中など、描画やゲームの更新を進めていけない状態なら
		// アプリは終了せずに、次のメッセージ処理へ進む
		// ここで描画と更新をやめるのでFPSを計測
		if (!CanUpdateFrame()) 
		{
			m_fpsController.EndFrame();
			continue; 
		}

		EndFrame();
	}

	// もしゲームデータがセーブされていなくても変更が適用されるべき項目を自動セーブする
	SaveCONFIG();
}

void Application::LoadCONFIG()
{
	m_window.LoadCONFIG		  ();
	m_fpsController.LoadCONFIG();
}

void Application::PostLoadCONFIG()
{
	m_window.PostLoadCONFIG(k_windowClassName, k_titleName);
}

bool Application::BeginFrame()
{
	// FPSの計測開始
	m_fpsController.BeginFrame();

	if (!m_window.ProcessMessages()) { return false; }

	// ウィンドウズハンドルを所持していないかエスケープキーを押されたらreturn
	if (GetAsyncKeyState(VK_ESCAPE) ||
		!m_window.HasHWND())
	{
		return false;
	}

	return true;
}
void Application::EndFrame()
{
	m_fpsController.EndFrame();

	UpdateWindowTitleBar();
}

void Application::ProcessResizeRequest()
{
	m_window.ClearResizeRequest();
}

bool Application::CanUpdateFrame() const
{
	// 最小化中なら描画をしないし、勝手にゲーム
	// を更新されても困るのでfalseを返す
	if (m_window.IsMinimized()) { return false; }

	return true;
}

void Application::SaveCONFIG() const
{
	m_window.SaveCONFIG		  ();
	m_fpsController.SaveCONFIG();
}

void Application::UpdateWindowTitleBar() const
{
	// タイトル名 + FPSのテキスト
	const auto& l_text = std::format("{} : {}", k_titleName, static_cast<int>(m_fpsController.GetVALCurrentFPS()));

	// ウィンドウバーに表示
	SetWindowTextA(m_window.GetREFHWND(), l_text.c_str());
}