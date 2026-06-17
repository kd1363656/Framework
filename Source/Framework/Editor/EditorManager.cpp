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

	const auto& l_graphicsManager= Graphics::GraphicsManager::GetInstance ();
	const auto& l_deviceWrapper	 = l_graphicsManager.GetREFDevice		  ();
	const auto& l_device		 = l_deviceWrapper.GetREFDevice			  ();
	const auto& l_renderer		 = l_graphicsManager.GetREFRenderer		  ();
	
	FWK_ASSERT_RETURN_IF_FAILED(!l_device, "Deviceが無効のため、ImGuiの初期化処理にに失敗しました。");

	const auto& l_directCommandQueue = l_renderer.GetREFDirectCommandQueue    ();
	const auto& l_commandQueue       = l_directCommandQueue.GetREFCommandQueue();

	// ImGui用SRVDescriptorPoolの作成
	FWK_ASSERT_RETURN_IF_FAILED(!CreateImGuiSRVDescriptorPool(l_deviceWrapper), "ImGui用SRVDescriptorPoolの作成に失敗したため、ImGuiの初期化処理に失敗しました。");

	const auto& l_imGuiShaderVisibleDescriptorHeap = m_imGuiSRVDescriptorPool.GetREFShaderVisibleDescriptorHeap();

	FWK_ASSERT_RETURN_IF_FAILED(!l_imGuiShaderVisibleDescriptorHeap, "ImGui用SRVDescriptorHeapのShaderVisibleなHeapラッパーが無効のため、ImGuiの初期化処理に失敗しました。");

	const auto& l_imGuiDescriptorHeap = l_imGuiShaderVisibleDescriptorHeap->GetREFDescriptorHeap();

	FWK_ASSERT_RETURN_IF_FAILED(!l_imGuiDescriptorHeap, "ImGui用SRVDescriptorHeapが無効のため、ImGuiの初期化処理に失敗しました。");
	FWK_ASSERT_RETURN_IF_FAILED(!l_commandQueue,        "コマンドキューが無効のため、ImGuiの初期化処理に失敗しました。");

	const auto& l_frameResourceList = l_renderer.GetREFFrameResourceList();

	FWK_ASSERT_RETURN_IF_FAILED(l_frameResourceList.empty(), "フレームリソースリストが空のため、ImGuiの初期化処理にに失敗しました。");

	// ImGuiのバージョンをチェックして、ImGuiContextを作成する
	IMGUI_CHECKVERSION  ();
	ImGui::CreateContext();

	auto& l_io = ImGui::GetIO();

	l_io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	// スタイル設定
	EditorStyle::ApplyFont           ();
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

	// ImGuiがSRVを確保するときに使うShaderVisibleなSRVDescriptorHeapを設定する
	l_initINFO.SrvDescriptorHeap = l_imGuiDescriptorHeap.Get();

	// ImGuiがSRVを確保するときに使うShaderVisibleなSRVDescriptorHeapを設定する
	l_initINFO.SrvDescriptorAllocFn = &EditorManager::AllocateSRVDescriptor;

	// ImGuiがSRVを開放するときに呼ばれる解放関数を設定する
	l_initINFO.SrvDescriptorFreeFn = &EditorManager::ReleaseSRVDescriptor;

	// WIN32用ImGuiバックエンドを初期化する
	// ImGui_ImplWin32_Init(入力を受け取る対象ウィンドウハンドル);
	FWK_ASSERT_RETURN_IF_FAILED(!ImGui_ImplWin32_Init(a_hwnd),      "IMGUI_IMPLWIN32_INITに失敗したため、ImGuiの初期化処理にに失敗しました。");

	// DirectX12用ImGuiバックエンドを初期化する
	// ImGui_ImplDX12_Init(DirectX12用初期化情報);
	FWK_ASSERT_RETURN_IF_FAILED(!ImGui_ImplDX12_Init (&l_initINFO), "ImGui_ImplDX12_Initに失敗したため、ImGuiの初期化処理にに失敗しました。");

	if (!m_logEditorWindow)
	{
		m_logEditorWindow = std::make_unique<Editor::LogEditorWindow>();
	}
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

		l_editorWindow->PostDeserialize();
	}
}

void FWK::Editor::EditorManager::DrawEdtor() const
{
	// ログウィンドウがインスタンス化されていなければ実行しない
	if (!m_logEditorWindow) { return; }

	const auto& l_graphicsManager    = Graphics::GraphicsManager::GetInstance ();
	const auto& l_renderer			 = l_graphicsManager.GetREFRenderer		  ();
	const auto& l_directCommandList  = l_renderer.GetREFDirectCommandList	  ();

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
	l_directCommandList.SetupDescriptorHeap(m_imGuiSRVDescriptorPool);

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

FWK::TypeAlias::DescriptorIndex FWK::Editor::EditorManager::AllocateImGuiSRVDescriptorIndex()
{
	const auto l_imGuiSRVDescriptorIndex = m_imGuiSRVDescriptorPool.Allocate();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_imGuiSRVDescriptorIndex == Constant::k_invalidDescriptorIndex, "ImGui用SRVDescriptorIndexの確保に失敗しました。", Constant::k_invalidDescriptorIndex);

	return l_imGuiSRVDescriptorIndex;
}

void FWK::Editor::EditorManager::ReleaseImGuiSRVDescriptorIndex(const TypeAlias::DescriptorIndex a_imGuiSRVDescriptorIndex)
{
	if (a_imGuiSRVDescriptorIndex == Constant::k_invalidDescriptorIndex) { return; }

	m_imGuiSRVDescriptorPool.Release(a_imGuiSRVDescriptorIndex);
}

bool FWK::Editor::EditorManager::UpdateImGuiSRVDescriptorFromMainSRVDescriptor(const TypeAlias::DescriptorIndex a_sourceSRVDescriptorIndex, const TypeAlias::DescriptorIndex a_imGuiSRVDescriptorIndex)
{
	const auto& l_graphicsManager   = Graphics::GraphicsManager::GetInstance   ();
	const auto& l_resourceContext   = l_graphicsManager.GetREFResourceContext  ();
	const auto& l_srvDescriptorPool = l_resourceContext.GetREFSRVDescriptorPool();

	return CopySRVDescriptorToImGuiSRVDescriptor(l_srvDescriptorPool, a_sourceSRVDescriptorIndex, a_imGuiSRVDescriptorIndex);
}

ImTextureID FWK::Editor::EditorManager::FetchVALImGuiTextureID(const TypeAlias::DescriptorIndex a_imGuiSRVDescriptorIndex) const
{
	if (a_imGuiSRVDescriptorIndex == Constant::k_invalidDescriptorIndex) { return {}; }

	const auto l_gpuDescriptorHandle = m_imGuiSRVDescriptorPool.FetchVALGPUDescriptorHandle(a_imGuiSRVDescriptorIndex);

	return l_gpuDescriptorHandle.ptr;
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
	FWK_ASSERT_RETURN_IF_FAILED(!a_info         ||
								!a_outCPUHandle ||
							    !a_outGPUHandle,
								"ImGui用のSRVDescriptorIndexの確保に失敗しました。");

	auto* const l_editorManager = static_cast<EditorManager*>(a_info->UserData);

	FWK_ASSERT_RETURN_IF_FAILED(!l_editorManager, "EditorManagerが無効のため、ImGui用SRVDescriptorIndexの確保に失敗しました。");

	const auto l_imGuiSRVDescriptorIndex = l_editorManager->m_imGuiSRVDescriptorPool.Allocate();

	FWK_ASSERT_RETURN_IF_FAILED(l_imGuiSRVDescriptorIndex == Constant::k_invalidDescriptorIndex, "ImGui用SRVDescriptorIndex確保に失敗しました。");

	*a_outCPUHandle = l_editorManager->m_imGuiSRVDescriptorPool.FetchVALShaderVisibleCPUDescriptorHandle(l_imGuiSRVDescriptorIndex);
	*a_outGPUHandle = l_editorManager->m_imGuiSRVDescriptorPool.FetchVALGPUDescriptorHandle			    (l_imGuiSRVDescriptorIndex);
	
	// ReleaseSRVDescriptor()ではGPUHandleが返ってくる
	// そのためCPUHandle.ptrではなくGPUHandle.ptrをキーにして保存する
	l_editorManager->m_imGuiSRVDescriptorIndexMap.try_emplace(a_outGPUHandle->ptr, l_imGuiSRVDescriptorIndex);
}

void FWK::Editor::EditorManager::ReleaseSRVDescriptor(ImGui_ImplDX12_InitInfo * a_info, D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE a_gpuHandle)
{
	if (!a_info) { return; }

	auto* const l_editorManager = static_cast<EditorManager*>(a_info->UserData);

	if (!l_editorManager) { return; }

	const auto& l_itr = l_editorManager->m_imGuiSRVDescriptorIndexMap.find(a_gpuHandle.ptr);

	if (l_itr == l_editorManager->m_imGuiSRVDescriptorIndexMap.end()) { return; }

	l_editorManager->m_imGuiSRVDescriptorPool.Release  (l_itr->second);
	l_editorManager->m_imGuiSRVDescriptorIndexMap.erase(l_itr);
}

bool FWK::Editor::EditorManager::CreateImGuiSRVDescriptorPool(const Graphics::Device& a_device)
{
	auto& l_descriptorIndexAllocator = m_imGuiSRVDescriptorPool.GetMutableREFDescriptorIndexAllocator();

	l_descriptorIndexAllocator.SetIndexCapacity(k_imguiSRVDescriptorCapacity);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_imGuiSRVDescriptorPool.Create(a_device), "ImGui用SRVDescriptorPoolの作成に失敗しました。", false);

	return true;
}

bool FWK::Editor::EditorManager::CopySRVDescriptorToImGuiSRVDescriptor(const TypeAlias::SRVDescriptorPool& a_sourceSRVDescriptorPool, const TypeAlias::DescriptorIndex a_sourceSRVDescriptorIndex, const TypeAlias::DescriptorIndex a_imGuiSRVDescriptorIndex) const
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_sourceSRVDescriptorIndex == Constant::k_invalidDescriptorIndex, "コピー元SRVDescriptorIndexが無効のため、ImGui用SRVDescriptorの更新に失敗しました。",       false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_imGuiSRVDescriptorIndex  == Constant::k_invalidDescriptorIndex, "コピー先ImGui用SRVDescriptorIndexが無効のため、ImGui用SRVDescriptorの更新に失敗しました。", false);

	const auto& l_graphicsManager = Graphics::GraphicsManager::GetInstance();
	const auto& l_deviceWrapper   = l_graphicsManager.GetREFDevice		  ();
	const auto& l_device		  = l_deviceWrapper.GetREFDevice			  ();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_device, "Deviceが無効のため、ImGui用SRVDescriptorの更新に失敗しました。", false);

	const auto l_sourceCPUDescriptorHandle      = a_sourceSRVDescriptorPool.FetchVALCPUDescriptorHandle(a_sourceSRVDescriptorIndex);
	const auto l_destinationCPUDescriptorHandle = m_imGuiSRVDescriptorPool.FetchVALCPUDescriptorHandle (a_imGuiSRVDescriptorIndex);

	// SRVDescriptorだけをコピー
	l_device->CopyDescriptorsSimple(k_copySRVDescriptorCount,
									l_destinationCPUDescriptorHandle,
									l_sourceCPUDescriptorHandle,
									D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// ImGui描画ではShaderVisible側のHeapを使うため、
	// CPU側へコピーしたDescriptorをShaderVisible側にも反映する
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_imGuiSRVDescriptorPool.CopyCPUDescriptorToShaderVisibleDescriptor(l_deviceWrapper, a_imGuiSRVDescriptorIndex), "ImGui用SRVDescriptorのShaderVisibleHeapへの反映に失敗しました。", false);

	return true;
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

	for (const auto& [l_gpuHandle, l_descriptorIndex] : m_imGuiSRVDescriptorIndexMap)
	{
		m_imGuiSRVDescriptorPool.Release(l_descriptorIndex);
	}
	
	m_imGuiSRVDescriptorIndexMap.clear();

	m_logEditorWindow.reset();
}