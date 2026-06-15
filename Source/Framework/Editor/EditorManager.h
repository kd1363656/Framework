#pragma once

namespace FWK::Editor
{
	class EditorManager final : public Utility::SingletonBase<EditorManager>
	{
	private:

		using SRVDescriptorIndexMap = std::unordered_map<UINT64, TypeAlias::DescriptorIndex>;

		friend class SingletonBase<EditorManager>;

		 EditorManager();
		~EditorManager() override;

	public:

		void INIT      (const HWND& a_hwnd);
		void LoadCONFIG();

		void DrawEdtor();

		void SaveCONFIG() const;

	private:

		static void AllocateSRVDescriptor(ImGui_ImplDX12_InitInfo* a_info, D3D12_CPU_DESCRIPTOR_HANDLE* a_outCPUHandle, D3D12_GPU_DESCRIPTOR_HANDLE* a_outGPUHandle);

		static void ReleaseSRVDescriptor(ImGui_ImplDX12_InitInfo* a_info, D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE a_gpuHandle);

		void DrawDockingSpace() const;

		void Release();

		const std::filesystem::path k_configFileIOPath = "Asset/Data/CONFIG/Editor/EditorCONFIG.json";

		static constexpr const char* k_dockingWindowName = "DockSpace";
		static constexpr const char* k_dockingSpaceName  = "DockSpace";

		static constexpr float k_dockingWindowRounding   = 0.0F;
		static constexpr float k_dockingWindowBorderSize = 0.0F;

		static constexpr int k_dockingStyleVarPopCount = 2;

		SRVDescriptorIndexMap m_srvDescriptorIndexMap = {};

		bool m_isInitialized = false;
		bool m_isValidEditor = false;
	};
}