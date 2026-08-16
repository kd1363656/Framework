#pragma once

namespace FWK::Editor
{
	class LogEditorWindow final
	{
	public:

		 LogEditorWindow();
		~LogEditorWindow();

		void Draw();

		void AddLog(const TypeAlias::Math::Color& a_textColor, const std::string& a_text);
		
	private:

		void DrawEditorOptions();
		void DrawLog          ();

		void ClearLog();

		static constexpr std::string_view k_editorName             = "ログビュー";
		static constexpr std::string_view k_optionString           = "設定";
		static constexpr std::string_view k_optionAutoScrollString = "自動スクロール";
		static constexpr std::string_view k_optionClearString      = "クリア";
		static constexpr std::string_view k_optionCopyString       = "コピー";
		static constexpr std::string_view k_optionFilterString     = "フィルター";
		static constexpr std::string_view k_optionScrollString     = "スクロール";

		static constexpr float k_filterInputWidth    = -100.0F;
		static constexpr float k_scrollToBottomRatio =  1.0F;

		static constexpr int k_nextLineStartOffset  =  1;
		static constexpr int k_excludeNewLineOffset = -1;
		static constexpr int k_nextLineIndexOffset  =  1;

		static constexpr std::size_t k_ensureFirstLine = 0ULL;

		static constexpr bool k_enableCanAutoScroll     = true;
		static constexpr bool k_enableCanScrollToBottom = true;

		std::vector<TypeAlias::Math::Color> m_textLineColorList;

		ImGuiTextBuffer m_textBuffer;
		ImGuiTextFilter m_textFilter;
		ImVector<int>   m_textLineOffsets;

		bool m_canAutoScroll;
		bool m_canScrollToBottom;
	};
}