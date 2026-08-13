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
	if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED))) { return Application::k_exitCodeCOMInitializeFailed; }

	Application::GetInstance().Execute();

	// COM解放
	CoUninitialize();

	return Application::k_exitCodeSuccess;
}

void Application::Execute()
{
	auto& l_physicsManager  = FWK::Physics::PhysicsManager::GetInstance  ();
	auto& l_graphicsManager = FWK::Graphics::GraphicsManager::GetInstance();
	auto& l_editorManager   = FWK::Editor::EditorManager::GetInstance    ();
	auto& l_sceneManager    = FWK::SceneManager::GetInstance			 ();
	
	l_graphicsManager.INIT();
	l_physicsManager.INIT ();

	LoadCONFIG                  ();
	l_graphicsManager.LoadCONFIG();
	l_editorManager.LoadCONFIG  ();
	l_physicsManager.LoadCONFIG ();
	
	PostLoadCONFIG					();
	l_graphicsManager.PostLoadCONFIG(m_window);


	l_editorManager.INIT		  (m_window.GetREFHWND());
	l_editorManager.PostLoadCONFIG();

	// 最初に読み込むべきシーンを読み込む
	l_sceneManager.LoadScene(k_firstLoadSceneFilepath);

	// while分に入る前にもう一度計測時間をリセットしておく
	m_fpsController.PostLoadCONFIG();

	while (true)
	{
		// ウィンドウメッセージやアプリケーションを終了するかの処理をしているので
		// falseが戻り値ならbreakする
		if (!BeginFrame()) { break; }

		// ウィンドウのリサイズ通知が来ていたらリサイズ処理を行う
		l_graphicsManager.ProcessWindowResizeRequest(m_window.GetREFResizeRequest());
		l_editorManager.ProcessWindowResizeRequest  (m_window.GetREFResizeRequest());

		// リサイズ要求フラグをクリア(サイズ変更時に一回だけ検知してほしいため)
		ClearWindowResizeRequest();

		// 最小化中など、描画やゲームの更新を進めていけない状態なら
		// アプリは終了せずに、次のメッセージ処理へ進む
		// ここで描画と更新をやめるのでFPSを計測
		if (!CanUpdateFrame()) 
		{
			m_fpsController.EndFrame();
			continue; 
		}

		// CharacterVirtualのDraw内容も受け取るため、ここでクリア
		l_physicsManager.ClearFrame();

		// 更新
		l_sceneManager.EarlyUpdate   ();
		l_sceneManager.Update        ();
		l_sceneManager.LateUpdate    ();
		l_sceneManager.PostLateUpdate();

		// JoltPhysicsのデバック命令をこのフレーム用のQueueへ集める
	    // ここで集めたLineListを、後続のPhysicsDebugPassがFinalColorへ描画する
		l_physicsManager.CollectPhysicsDebugDrawCommands();

		// 描画処理
		l_graphicsManager.BeginFrame();
		l_graphicsManager.Execute   ();

		// エディターの描画
		// (描画する際にレンダーターゲットのリソース状態がPresentではエディターの描画ができないため)
		l_editorManager.DrawEditor();

		l_graphicsManager.EndFrame();

		EndFrame();
	}

	// もしゲームデータがセーブされていなくても変更が適用されるべき項目を自動セーブする
	SaveCONFIG					();
	l_graphicsManager.SaveCONFIG();
	l_editorManager.SaveCONFIG  ();
	l_physicsManager.SaveCONFIG ();
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
		!m_window.GetREFHWND())
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

void Application::SaveCONFIG() const
{
	m_window.SaveCONFIG		  ();
	m_fpsController.SaveCONFIG();
}

void Application::ClearWindowResizeRequest()
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

void Application::UpdateWindowTitleBar() const
{
	// タイトル名 + FPSのテキスト
	const auto& l_text = std::format("{} : {}", k_titleName, static_cast<int>(m_fpsController.GetVALCurrentFPS()));

	// ウィンドウバーに表示
	SetWindowTextA(m_window.GetREFHWND(), l_text.c_str());
}