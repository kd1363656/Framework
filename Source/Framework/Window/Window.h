#pragma once

namespace FWK
{
	class Window final
	{
	public:

		struct ClientSize final
		{
			UINT m_width  = k_defaultWindowWidth;
			UINT m_height = k_defaultWindowHeight;
		};

		struct ResizeRequest final
		{
			ClientSize m_clientSize = { k_invalidClientWidth, k_invalidClientHeight };

			bool m_isRequested = false;
			bool m_isMinimized = false;
		};

		static constexpr UINT k_defaultWindowWidth  = 1280U;
		static constexpr UINT k_defaultWindowHeight = 720U;

		static constexpr UINT k_invalidClientWidth  = 0U;
		static constexpr UINT k_invalidClientHeight = 0U;

	public:

		 Window();
		~Window();

		void LoadCONFIG    ();
		void PostLoadCONFIG(const std::wstring& a_windowClassName, const std::string& a_titleName);

		bool ProcessMessages() const;

		void ClearResizeRequest();

		void SaveCONFIG() const;

		bool IsMinimized() const;

		void SetupStyle(const Enum::WindowStyle a_style);

		void SetStyle(const Enum::WindowStyle a_set) { m_style = a_set; }

		const auto& GetREFHWND() const { return m_hwnd; }

		const auto& GetREFClientSize   () const { return m_clientSize; }
		const auto& GetREFResizeRequest() const { return m_resizeRequest; }

		auto GetVALStyle() const { return m_style; }

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

		void ApplyClientSizeFromWMSize(const ClientSize& a_clientSize, const WPARAM& a_wPARAM);

		void ApplyWindowStyle();

		void ApplyNormalWindowStyle();
		
		void ApplyBorderlessFullScreenWindowStyle();

		void StoreNormalWindowRECT();

		void RequestResizeFromClientSize(const ClientSize& a_clientSize);

		HINSTANCE FetchVALInstanceHandle() const;

		DWORD FetchVALWindowStyle() const;

		ClientSize FetchVALCurrentClientSize() const;

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

		const std::filesystem::path k_configFileIOPath = "Asset/Data/CONFIG/Window/WindowCONFIG.json";

		Converter::WindowJsonConverter m_jsonConverter = {};

		RECT m_normalWindowRECT = {};

		ClientSize    m_clientSize;
		ResizeRequest m_resizeRequest;

		HWND m_hwnd;

		Enum::WindowStyle m_style;
	};
}