#pragma once

namespace FWK::Editor
{
	class LogEditorWindow final : public EditorWindowBase
	{
	public:

		 LogEditorWindow()          = default;
		~LogEditorWindow() override = default;

		bool IsAllowCreateInList() const override { return false; }

		void INIT() override;
		void Draw() override;

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

		ImGuiTextBuffer m_textBuffer        = {};
		ImGuiTextFilter m_textFilter        = {};
		ImVector<int>   m_textLineOffsets   = {};

		bool m_canAutoScroll     = k_enabelCanAutoScroll;
		bool m_canScrollToBottom = k_enabelCanScrollToBottom;

		FWK_DEFINE_TYPE_INFO(LogEditorWindow, EditorWindowBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::SharedFactoryEditorWindow, FWK::Editor::LogEditorWindow)