#pragma once

namespace FWK
{
	class Window final
	{
	public:

		 Window();
		~Window();

		void LoadCONFIG    ();
		void PostLoadCONFIG(const std::wstring& a_windowClassName, const std::string& a_titleName);

		bool ProcessMessages() const;

		void ClearResizeRequest();

		void SaveCONFIG() const;

		bool IsMinimized() const;

		void SetupWindowStyleTag(const TypeAlias::TagType a_windowStyleTag);

		void SetWindowStyleTag(const TypeAlias::TagType a_set) { m_windowStyleTag = a_set; }

		const auto& GetREFHWND() const { return m_hwnd; }

		const auto& GetREFClientSize   () const { return m_clientSize; }
		const auto& GetREFResizeRequest() const { return m_resizeRequest; }

		auto GetVALWindowStyleTag() const { return m_windowStyleTag; }

	private:

		// Win32APIに渡すウィンドウプロシージャは通常のメンバ関数では渡せないため、
		// static関数として定義して呼び出しの入口にする
		static LRESULT CALLBACK CallWindowProcedure(const HWND   a_hwnd,
												    const UINT   a_message,
												    const WPARAM a_wPARAM,
												    const LPARAM a_lPARAM);

		LRESULT CALLBACK WindowProcedure(const HWND   a_hwnd,
										 const UINT   a_message,
										 const WPARAM a_wPARAM,
										 const LPARAM a_lPARAM);

		bool CreateWindowInstance(const std::wstring& a_windowClassName, const std::string& a_titleName);

		void SetupNormalWindowClientSize();

		void Release();

		void ApplyClientSizeFromWMSize(const Struct::ClientSize& a_clientSize, const WPARAM& a_wPARAM);

		void ApplyWindowStyle();

		void ApplyNormalWindowStyle();
		
		void ApplyBorderlessFullScreenWindowStyle();

		void StoreNormalWindowRECT();

		void RequestResizeFromClientSize(const Struct::ClientSize& a_clientSize);

		HINSTANCE FetchVALInstanceHandle() const;

		DWORD FetchVALWindowStyle() const;

		FWK::Struct::ClientSize FetchVALCurrentClientSize() const;

		const std::filesystem::path k_configFileIOPath = "Asset/Data/CONFIG/Window/WindowCONFIG.json";

		// ウィンドウのタイトルバー、最小化、最大化機能を持たせウィンドウのサイズ変更機能を除外したスタイル
		static constexpr std::wstring_view k_windowInstancePropertyName = L"GameWindowInstance";

		static constexpr LRESULT k_windowProcedureHandledResult = 0;

		static constexpr LONG k_clientRECTLeft = 0L;
		static constexpr LONG k_clientRECTTop  = 0L;

		// 通常ウィンドウ
		// タイトルバー、幅、最小化、最大化を持つ
		static constexpr DWORD k_generalWindowStyle = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME;

		// 枠なしウィンドウ
		// ボーダーレスフルスクリーンで使う
		static constexpr DWORD k_borderlessFullScreenWindowStyle = WS_POPUP;

		static constexpr UINT k_windowStyle = CS_HREDRAW | CS_VREDRAW;

		static constexpr UINT k_timeResolutionMS = 1U;

		static constexpr UINT k_msgFilterMIN		  = 0U;
		static constexpr UINT k_msgFilterMAX		  = 0U;
		static constexpr UINT k_wmCreateHandledResult = 0U;

		static constexpr int k_classExtraBytes  = 0;
		static constexpr int k_windowExtraBytes = 0;

		static constexpr int k_defaultWindowPositionX = 0;
		static constexpr int k_defaultWindowPositionY = 0;

		static constexpr int k_quitExitCode = 0;

		HWND m_hwnd;

		Converter::WindowJsonConverter m_jsonConverter = {};

		RECT m_normalWindowRECT = {};

		Struct::ClientSize			m_clientSize;
		Struct::WindowResizeRequest m_resizeRequest;

		TypeAlias::TagType m_windowStyleTag;
	};
}