#pragma once

namespace FWK
{
	class Window final
	{
	public:

		 Window();
		~Window();

		void PostLoadCONFIG(const std::wstring& a_windowClassName, const std::string& a_titleName);

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

		void SetupClientSize();

		void Release();

		void ApplyClientSizeFromWMSize(const Struct::WindowSize& a_windowSize, const WPARAM& a_wPARAM);

		HINSTANCE FetchVALInstanceHandle() const;

		DWORD FetchVALWindowStyle() const;

		// ウィンドウのタイトルバー、最小化、最大化機能を持たせウィンドウのサイズ変更機能を除外したスタイル
		static constexpr std::wstring_view k_windowInstancePropertyName = L"GameWindowInstance";

		static constexpr LRESULT k_windowProcedureHandledResult = 0;

		static constexpr DWORD k_generalWindowStyle = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME;

		static constexpr UINT k_wmCreateHandledResult = 0U;

		static constexpr UINT k_windowStyle = CS_HREDRAW | CS_VREDRAW;

		static constexpr int k_classExtraBytes  = 0;
		static constexpr int k_windowExtraBytes = 0;

		static constexpr int k_defaultWindowPositionX = 0;
		static constexpr int k_defaultWindowPositionY = 0;

		static constexpr int k_quitExitCode = 0;

		HWND m_hwnd;

		Struct::WindowSize m_windowSize = {};

		TypeAlias::TagType m_windowStyleTag = Utility::GetVALTag<Tag::WindowStyleNormalTag>();

		bool m_isRequestResize = false;
		bool m_isMinimized     = false;
	};
}