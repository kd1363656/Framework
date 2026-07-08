#pragma once

namespace FWK::Editor
{
	class EditorManager final : public Utility::SingletonBase<EditorManager>
	{
	private:

		using ImGuiSRVDescriptorIndexMap = std::unordered_map<UINT64, TypeAlias::DescriptorIndex>;
		using EditorWindowMap			 = std::unordered_map<TypeAlias::StaticTypeID, std::weak_ptr<EditorWindowBase>>;

		friend class SingletonBase<EditorManager>;

		 EditorManager();
		~EditorManager() override;

	public:

		void INIT          (const HWND& a_hwnd);
		void LoadCONFIG    ();
		void PostLoadCONFIG() const;

		void DrawEditor();
		
		void SaveCONFIG() const;
	
		void ProcessWindowResizeRequest(const Struct::WindowResizeRequest& a_windowResizeRequest);

		bool CopyGraphicsSRVDescriptor(const TypeAlias::CBVSRVUAVDescriptorPool& a_sourceCBVSRVUAVDescriptorPool, const TypeAlias::DescriptorIndex a_sourceSRVDescriptorIndex, const TypeAlias::DescriptorIndex a_imGuiSRVDescriptorIndex) const;

		TypeAlias::DescriptorIndex AllocateImGuiSRVDescriptorIndex();

		void ReleaseImGuiSRVDescriptorIndex(const TypeAlias::DescriptorIndex a_imGuiSRVDescriptorIndex);

		ImTextureID FetchVALImGuiTextureID(const TypeAlias::DescriptorIndex a_imGuiSRVDescriptorIndex) const;

		template <class... Args>
		void AddLog(const std::source_location& a_location, const std::string_view& a_format, Args&&... a_args)
		{
			// a_formatとa_args...を使って、ログ本文の文字列を作成する
			// 例 : FWK_ADD_LOG("HP = {}, Name = {}", 100, "Player");
			// この場合はa_format = "HP = {}, Name = {}" a_args... = 100, "Player"
			// std::make_format_args(...)はstd::vformatに渡すための「フォーマット用引数リスト」を作成する。
			// a_args...は、受け取った可変長引数を1つずつ展開してstd::make_format_argsに渡している。
			// 最終的にl_messageには、"HP = 100, Name = Player"のような文字列が入る
			const std::string l_message = std::vformat(a_format, std::make_format_args(a_args...));

			// 呼びだし元情報をつけてログ本文を記述
			m_logEditorWindow.AddLog("[%s : %u][%s]\n%s\n",
									 a_location.file_name(),
									 a_location.line(),
									 a_location.function_name(),
									 l_message.c_str());
		}
	
		void AddEditorWindow(const std::shared_ptr<EditorWindowBase>& a_editorWindow);

		template <Concept::IsDerivedEditorWindowBaseConcept WindowType>
		std::weak_ptr<WindowType> FetchWindowEditor() const
		{
			const auto l_staticTypeID = WindowType::GetREFTypeINFO().k_staticTypeID;

			const auto& l_itr = m_editorWindowMap.find(l_staticTypeID);

			if (l_itr == m_editorWindowMap.end()) { return {}; }

			auto l_editorWindow = l_itr->second.lock();

			if (!l_editorWindow) { return {}; }

			return std::static_pointer_cast<WindowType>(l_editorWindow);
		}

		void SetIsDisableDrawEditor(const bool a_set) { m_isDisableDrawEditor = a_set; }

		const auto& GetREFEditorWindowList() const { return m_editorWindowList; }

		const auto& GetREFMainMenubar() const { return m_mainMenubar; }

		auto& GetMutableREFMainMenubar() { return m_mainMenubar; }

		bool GetVALIsDisableDrawEditor() const { return m_isDisableDrawEditor; }

	private:

		static void AllocateSRVDescriptor(ImGui_ImplDX12_InitInfo* a_info, D3D12_CPU_DESCRIPTOR_HANDLE* a_outCPUHandle, D3D12_GPU_DESCRIPTOR_HANDLE* a_outGPUHandle);

		static void ReleaseSRVDescriptor(ImGui_ImplDX12_InitInfo* a_info, D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE a_gpuHandle);

		bool CreateImGuiSRVDescriptorPool(const Graphics::Device& a_device);

		void DrawDockingSpace() const;
		void DrawEditorWindow();

		void Release();

		const std::filesystem::path k_configFileIOPath = "Asset/Data/CONFIG/Editor/EditorCONFIG.json";

		static constexpr const char* k_dockingWindowName = "DockSpace";
		static constexpr const char* k_dockingSpaceName  = "DockSpace";

		static constexpr float k_dockingWindowRounding   = 0.0F;
		static constexpr float k_dockingWindowBorderSize = 0.0F;

		static constexpr size_t k_logBufferSize = 1024ULL;

		static constexpr TypeAlias::DescriptorIndex k_imguiSRVDescriptorCapacity = 2560U;
	
		static constexpr UINT k_copySRVDescriptorCount = 1U;

		static constexpr int k_dockingStyleVarPopCount = 2;

		TypeAlias::CBVSRVUAVDescriptorPool m_imGuiCBVSRVUAVDescriptorPool;

		ImGuiSRVDescriptorIndexMap m_imGuiSRVDescriptorIndexMap;

		EditorWindowMap m_editorWindowMap;

		std::vector<std::shared_ptr<FWK::Editor::EditorWindowBase>> m_editorWindowList;

		Editor::MainMenubarEditor m_mainMenubar;
		Editor::LogEditorWindow   m_logEditorWindow;

		Converter::EditorManagerJsonConverter m_jsonConverter;

		bool m_isInitialized;
		bool m_isDisableDrawEditor;
	};
}

// __VA_OPT(,)は可変長引数があるときだけ"","を追加するためのC++20の機能
#define FWK_ADD_LOG(Format , ...)																						 \
do																														 \
{																														 \
	FWK::Editor::EditorManager::GetInstance().AddLog(std::source_location::current(), Format __VA_OPT__(,) __VA_ARGS__); \
}																														 \
while(false)