#pragma once

namespace FWK::Editor
{
	class LogEditorWindow final
	{
	public:

		 LogEditorWindow();
		~LogEditorWindow();

		void Draw();

		void AddLog(const char* a_format , ...) IM_FMTARGS(2);

	private:

		void DrawEditorOptions();
		void DrawLog          ();

		void ClearLog();

		static constexpr float k_filterInputWidth    = -100.0F;
		static constexpr float k_scrollToBottomRatio =    1.0F;

		static constexpr int k_nextLineStartOffset  =  1;
		static constexpr int k_excludeNewLineOffset = -1;
		static constexpr int k_nextLineIndexOffset  =  1;

		static constexpr std::size_t k_ensureFirstLine = 0ULL;

		static constexpr bool k_enabelCanAutoScroll     = true;
		static constexpr bool k_enabelCanScrollToBottom = true;

		ImGuiTextBuffer m_textBuffer;
		ImGuiTextFilter m_textFilter;
		ImVector<int>   m_textLineOffsets;

		bool m_canAutoScroll;
		bool m_canScrollToBottom;
	};
}