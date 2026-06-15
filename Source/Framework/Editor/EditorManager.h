#pragma once

namespace FWK::Editor
{
	class EditorManager final : public Utility::SingletonBase<EditorManager>
	{
	private:

		using SRVDescriptorIndexMap = std::unordered_map<UINT64, TypeAlias::DescriptorIndex>;
		using EditorWindowMap       = std::unordered_map<TypeAlias::StaticTypeID, std::weak_ptr<EditorWindowBase>>;

		friend class SingletonBase<EditorManager>;

		 EditorManager();
		~EditorManager() override;

	public:

		void INIT          (const HWND& a_hwnd);
		void LoadCONFIG    ();
		void PostLoadCONFIG() const;

		void DrawEdtor() const;
		
		void SaveCONFIG() const;

		void AddLog(const std::source_location& a_location, const char* a_format, ...);

		void AddEditorWindow(const std::shared_ptr<EditorWindowBase>& a_editorWindow);

		template <Concept::IsDerivedEditorWindowBaseConcept WindowType>
		std::weak_ptr<WindowType> FetchWindowEditor() const
		{
			const TypeAlias::StaticTypeID l_staticTypeID = WindowType::GetTypeINFO().k_staticTypeID;

			const auto& l_itr = m_editorWindowMap.find(l_staticTypeID);

			if (l_itr == m_editorWindowMap.end()) { return {}; }

			auto l_editorWindow = l_itr->second.lock();

			if (!l_editorWindow) { return {}; }

			return std::static_pointer_cast<WindowType>(l_editorWindow);
		}

		const auto& GetREFEditorWindowList() const { return m_editorWindowList; }

	private:

		static void AllocateSRVDescriptor(ImGui_ImplDX12_InitInfo* a_info, D3D12_CPU_DESCRIPTOR_HANDLE* a_outCPUHandle, D3D12_GPU_DESCRIPTOR_HANDLE* a_outGPUHandle);

		static void ReleaseSRVDescriptor(ImGui_ImplDX12_InitInfo* a_info, D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE a_gpuHandle);

		void DrawDockingSpace() const;
		void DrawEditorWindow() const;

		void Release();

		const std::filesystem::path k_configFileIOPath = "Asset/Data/CONFIG/Editor/EditorCONFIG.json";

		static constexpr const char* k_dockingWindowName = "DockSpace";
		static constexpr const char* k_dockingSpaceName  = "DockSpace";

		static constexpr float k_dockingWindowRounding   = 0.0F;
		static constexpr float k_dockingWindowBorderSize = 0.0F;

		static constexpr size_t k_logBufferSize = 1024ULL;

		static constexpr int k_dockingStyleVarPopCount = 2;

		std::unique_ptr<Editor::LogEditorWindow> m_logEditorWindow = nullptr;

		SRVDescriptorIndexMap m_srvDescriptorIndexMap = {};
		EditorWindowMap       m_editorWindowMap       = {};

		std::vector<std::shared_ptr<FWK::Editor::EditorWindowBase>> m_editorWindowList = {};

		Converter::EditorManagerJsonConverter m_jsonConverter = {};

		bool m_isValidEditor = false;
	};
}

// const char8_t*をconst char*として扱うマクロ(c++20から使えなくなってるので)
#if defined(__cpp_char8_t)
	#define U8(X) ((const char *)u8##X)
#else
	#define U8(x) u8##x
#endif

#define FWK_ADD_LOG(Format , ...)																							  \
do																															  \
{																															  \
	FWK::Editor::EditorManager::GetInstance().AddLog(std::source_location::current(), U8(Format) __VA_OPT__(, ) __VA_ARGS__); \
} while(false)