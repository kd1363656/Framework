#include "EditorManager.h"

FWK::Editor::EditorManager::EditorManager() = default;
FWK::Editor::EditorManager::~EditorManager()
{
	Release();
}

void FWK::Editor::EditorManager::INIT(const HWND& a_hwnd)
{
	// ログウィンドウがインスタンス化されていたら実行しない
	if (m_logEditorWindow) { return; }

	const auto& l_graphicsManager    = Graphics::GraphicsManager::GetInstance ();
	const auto& l_device		     = l_graphicsManager.GetREFDevice		  ().GetREFDevice();
	const auto& l_renderer			 = l_graphicsManager.GetREFRenderer		  ();
	const auto& l_resourceContext	 = l_graphicsManager.GetREFResourceContext();
	
	FWK_ASSERT_RETURN_IF_FAILED(!l_device, "Deviceが無効のため、ImGuiの初期化処理にに失敗しました。");

	const auto& l_directCommandQueue = l_renderer.GetREFDirectCommandQueue    ();
	const auto& l_commandQueue       = l_directCommandQueue.GetREFCommandQueue();

	FWK_ASSERT_RETURN_IF_FAILED(!l_commandQueue, "コマンドキューが無効のため、ImGuiの初期化処理にに失敗しました。");

	const auto& l_srvDescriptorPool			  = l_resourceContext.GetREFSRVDescriptorPool            ();
	const auto& l_shadervisibleDescriptorHeap = l_srvDescriptorPool.GetREFShaderVisibleDescriptorHeap();

	FWK_ASSERT_RETURN_IF_FAILED(!l_shadervisibleDescriptorHeap, "SRVDescriptorHeapのShaderVisibleなHeapラッパーが無効のため、ImGuiの初期化処理にに失敗しました。");

	const auto& l_descriptorHeap = l_shadervisibleDescriptorHeap->GetREFDescriptorHeap();

	FWK_ASSERT_RETURN_IF_FAILED(!l_descriptorHeap, "SRVDescriptorHeapのShaderVisibleなHeapが無効のため、ImGuiの初期化処理にに失敗しました。");

	const auto& l_frameResourceList = l_renderer.GetREFFrameResourceList();

	FWK_ASSERT_RETURN_IF_FAILED(l_frameResourceList.empty(), "フレームリソースリストが空のため、ImGuiの初期化処理にに失敗しました。");

	// ImGuiのバージョンをチェックして、ImGuiContextを作成する
	IMGUI_CHECKVERSION  ();
	ImGui::CreateContext();

	auto& l_io = ImGui::GetIO();

	l_io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	// スタイル設定
	EditorStyle::ApplyDefaultFont    ();
	EditorStyle::ApplySakuraDarkStyle();

	ImGui_ImplDX12_InitInfo l_initINFO = {};

	// DirectX12のDeviceを設定する
	l_initINFO.Device = l_device.Get();

	// ImGui内部のテクスチャアップロードなどで使うDirectCommandQueueを設定する
	l_initINFO.CommandQueue = l_commandQueue.Get();

	// GPUとCPUで並行して使うフレーム数を設定する
	l_initINFO.NumFramesInFlight = static_cast<int>(l_frameResourceList.size());

	// メイン描画のRTVフォーマットを設定する
	l_initINFO.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	// 深度のフォーマット
	l_initINFO.DSVFormat = DXGI_FORMAT_UNKNOWN;

	// コールバック内でEditorManagerを取り出せるようにする
	l_initINFO.UserData = this;

	// ImGuiがSRVを保証するときに使うShaderVisibleなSRVDescriptorHeapを設定する
	l_initINFO.SrvDescriptorHeap = l_descriptorHeap.Get();

	// ImGuiがSRVを確保するときに使うShaderVisibleなSRVDescriptorHeapを設定する
	l_initINFO.SrvDescriptorAllocFn = &EditorManager::AllocateSRVDescriptor;

	// ImGuiがsrvを開放するときに呼ばれる解放関数を設定する
	l_initINFO.SrvDescriptorFreeFn = &EditorManager::ReleaseSRVDescriptor;

	// WIN32用ImGuiバックエンドを初期化する
	// ImGui_ImplWind32_Init(入力を受け取る対象ウィンドウハンドル);
	FWK_ASSERT_RETURN_IF_FAILED(!ImGui_ImplWin32_Init(a_hwnd),      "IMGUI_IMPLWIN32_INITに失敗したため、ImGuiの初期化処理にに失敗しました。");

	// DirectX12用ImGuiバックエンドを初期化する
	// ImGui_ImplDX12_Init(DirectX12用初期化情報);
	FWK_ASSERT_RETURN_IF_FAILED(!ImGui_ImplDX12_Init (&l_initINFO), "ImGui_ImplDX12_Initに失敗したため、ImGuiの初期化処理にに失敗しました。");

	if (!m_logEditorWindow)
	{
		m_logEditorWindow = std::make_unique<Editor::LogEditorWindow>();
	}

	m_logEditorWindow->INIT();
}
void FWK::Editor::EditorManager::LoadCONFIG()
{
	const auto& l_rootJson = Utility::LoadJsonFile(k_configFileIOPath);

	if (l_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(l_rootJson, *this);
}
void FWK::Editor::EditorManager::PostLoadCONFIG() const
{
	// デシリアライズで追加したウィンドウの初期化
	for (const auto& l_editorWindow : m_editorWindowList)
	{
		if (!l_editorWindow) { continue; }

		l_editorWindow->INIT();
	}
}

void FWK::Editor::EditorManager::DrawEdtor() const
{
	// ログウィンドウがインスタンス化されていなければ実行しない
	if (!m_logEditorWindow) { return; }

	const auto& l_graphicsManager    = Graphics::GraphicsManager::GetInstance ();
	const auto& l_renderer			 = l_graphicsManager.GetREFRenderer		  ();
	const auto& l_directCommandList  = l_renderer.GetREFDirectCommandList	  ();

	const auto& l_resourceContext	= l_graphicsManager.GetREFResourceContext  ();
	const auto& l_srvDescriptorPool = l_resourceContext.GetREFSRVDescriptorPool();
	
	// DirectX12用ImGuiバックエンドのフレーム開始処理
	ImGui_ImplDX12_NewFrame();

	// WIN32用ImGuiバックエンドのフレーム開始処理
	ImGui_ImplWin32_NewFrame();

	// ImGuiのフレーム開始
	ImGui::NewFrame();

	DrawDockingSpace();
	DrawEditorWindow();

	// ImGuiの描画データを確定する
	ImGui::Render();

	// ImGuiのフォントテクスチャなどはSRVDescriptorHeapを使うため、描画前に設定する
	l_directCommandList.SetupDescriptorHeap(l_srvDescriptorPool);

	// ImGuiの描画コマンドを現在記録中のDirectCommandListへ積む
	// ImGui_ImplDX12_RenderDrawData(IMGUIが作成した描画データ、
	//								 描画命令を書き込むDirectCommandList);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), l_directCommandList.GetREFCommandList().Get());
}

void FWK::Editor::EditorManager::SaveCONFIG() const
{
	const auto& l_rootJson = m_jsonConverter.Serialize(*this);

	Utility::SaveJsonFile(l_rootJson, k_configFileIOPath);
}

void FWK::Editor::EditorManager::AddEditorWindow(const std::shared_ptr<EditorWindowBase>& a_editorWindow)
{
	FWK_ASSERT_RETURN_IF_FAILED(!a_editorWindow, "作成しようとしているEditorWindowが無効になっており、追加処理を行えませんでした。");

	// リスト内に保持してはいけないウィンドウを保持しない
	if (!a_editorWindow->IsAllowCreateInList()) { return; }

	const auto& l_staticID = a_editorWindow->GetREFRuntimeTypeINFO().k_staticTypeID;

	// 既に作成されているならばreturn
	if (m_editorWindowMap.contains(l_staticID)) { return; }

	m_editorWindowList.emplace_back(a_editorWindow);
	m_editorWindowMap.try_emplace  (l_staticID, a_editorWindow);
}

void FWK::Editor::EditorManager::AllocateSRVDescriptor(ImGui_ImplDX12_InitInfo * a_info, D3D12_CPU_DESCRIPTOR_HANDLE * a_outCPUHandle, D3D12_GPU_DESCRIPTOR_HANDLE * a_outGPUHandle)
{
	auto& l_graphicsManager    = Graphics::GraphicsManager::GetInstance		 ();
	auto& l_resourceContext	 = l_graphicsManager.GetMutableREFResourceContext();
	
	auto& l_srvDescriptorPool = l_resourceContext.GetMutableREFSRVDescriptorPool();

	FWK_ASSERT_RETURN_IF_FAILED(!a_info         ||
								!a_outCPUHandle ||
							    !a_outGPUHandle,
								"ImGui用のSRVDescriptorIndexの確保に失敗しました。");

	auto* const l_editorManager = static_cast<EditorManager*>(a_info->UserData);

	FWK_ASSERT_RETURN_IF_FAILED(!l_editorManager, "EditorManagerが無効のため、ImGui用SRVDescriptorIndexの確保に失敗しました。");

	const auto l_srvDescriptorIndex = l_srvDescriptorPool.Allocate();

	FWK_ASSERT_RETURN_IF_FAILED(l_srvDescriptorIndex == Constant::k_invalidDescriptorIndex, "ImGui用SRVDescriptorIndex確保に失敗しました。");

	*a_outCPUHandle = l_srvDescriptorPool.FetchVALShaderVisibleCPUDescriptorHandle(l_srvDescriptorIndex);
	*a_outGPUHandle = l_srvDescriptorPool.FetchVALGPUDescriptorHandle             (l_srvDescriptorIndex);
	
	l_editorManager->m_srvDescriptorIndexMap.try_emplace(a_outCPUHandle->ptr, l_srvDescriptorIndex);
}

void FWK::Editor::EditorManager::ReleaseSRVDescriptor(ImGui_ImplDX12_InitInfo * a_info, D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE a_gpuHandle)
{
	if (!a_info) { return; }

	auto* const l_editorManager = static_cast<EditorManager*>(a_info->UserData);

	if (!l_editorManager) { return; }

	auto& l_graphicsManager = Graphics::GraphicsManager::GetInstance        ();
	auto& l_resourceContext = l_graphicsManager.GetMutableREFResourceContext();

	auto& l_srvDescriptorPool = l_resourceContext.GetMutableREFSRVDescriptorPool();

	const auto& l_itr = l_editorManager->m_srvDescriptorIndexMap.find(a_gpuHandle.ptr);

	if (l_itr == l_editorManager->m_srvDescriptorIndexMap.end()) { return; }

	l_srvDescriptorPool.Release(l_itr->second);

	l_editorManager->m_srvDescriptorIndexMap.erase(l_itr);
}

void FWK::Editor::EditorManager::DrawDockingSpace() const
{
	// ImGuiDockingSpaceの作成
	// DockSpace全体を包むルートウィンドウ(親ウィンドウ)を画面サイズで作成
	ImGuiWindowFlags l_windowFlag = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

	const ImGuiViewport* l_viewPort	= ImGui::GetMainViewport();

	// メインウィンドウを取得してサイズ、描画位置を現在のウィンドウサイズに合わせる
	ImGui::SetNextWindowPos     (l_viewPort->WorkPos);
	ImGui::SetNextWindowSize    (l_viewPort->WorkSize);
	ImGui::SetNextWindowViewport(l_viewPort->ID);

	// 親ウィンドウとして振舞うためウィンドウ名を表示しない、折り畳みができない
	// 移動しない、他ウィンドウがこのウィンドウより上に描画されるようにする、
	// キーボードナビゲーションでこのウィンドウにフォーカスがいかないようにする
	l_windowFlag |= ImGuiWindowFlags_NoTitleBar            | 
					ImGuiWindowFlags_NoCollapse            | 
					ImGuiWindowFlags_NoMove                |
					ImGuiWindowFlags_NoBringToFrontOnFocus |
					ImGuiWindowFlags_NoNavFocus;

	// 親ウィンドウを角丸・枠なしにする設定
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,   k_dockingWindowRounding);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, k_dockingWindowBorderSize);

	// Begin(ウィンドウ名、
	//		 開閉状態を管理するboolポインタ(nullptrなら外部から閉じない)、
	//		 ウィンドウの挙動フラグ);
	ImGui::Begin(k_dockingWindowName, nullptr, l_windowFlag);

	ImGui::PopStyleVar(k_dockingStyleVarPopCount);

	// "DockSpace"の作成
	ImGuiID l_dockSpaceID = ImGui::GetID(k_dockingSpaceName);
	ImVec2  l_size        = {};

	ImGui::DockSpace(l_dockSpaceID, l_size, ImGuiDockNodeFlags_None);
	ImGui::End      ();
}
void FWK::Editor::EditorManager::DrawEditorWindow() const
{
	for (const auto& l_editorWindow : m_editorWindowList)
	{
		if (!l_editorWindow) { continue; }

		l_editorWindow->Draw();
	}

	if (m_logEditorWindow)
	{
		m_logEditorWindow->Draw();
	}
}

void FWK::Editor::EditorManager::Release()
{
	// ログウィンドウがインスタンス化されていなければ実行しない
	if (!m_logEditorWindow) { return; }
	
	ImGui_ImplDX12_Shutdown ();
	ImGui_ImplWin32_Shutdown();

	if (ImGui::GetCurrentContext())
	{
		ImGui::DestroyContext();
	}

	auto& l_graphicsManager = Graphics::GraphicsManager::GetInstance();
	auto& l_resourceContext = l_graphicsManager.GetMutableREFResourceContext();

	auto& l_srvDescriptorPool = l_resourceContext.GetMutableREFSRVDescriptorPool();

	for (const auto& [l_gpuHandlePTR, l_storageID] : m_srvDescriptorIndexMap)
	{
		l_srvDescriptorPool.Release(l_storageID);
	}

	m_srvDescriptorIndexMap.clear();

	m_logEditorWindow.reset();
}