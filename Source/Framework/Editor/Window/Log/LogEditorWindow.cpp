#include "LogEditorWindow.h"

FWK::Editor::LogEditorWindow::LogEditorWindow() : 
	m_textLineColorList(),

	m_textBuffer     (),
	m_textFilter     (),
	m_textLineOffsets(),

	m_canAutoScroll    (k_enableCanAutoScroll),
	m_canScrollToBottom(k_enableCanScrollToBottom)
{
	ClearLog();
}
FWK::Editor::LogEditorWindow::~LogEditorWindow() = default;

void FWK::Editor::LogEditorWindow::Draw()
{
	if (!ImGui::Begin(k_editorName.data()))
	{
		ImGui::End();

		return;
	}

	DrawEditorOptions();
	DrawLog          ();

	ImGui::End();
}

void FWK::Editor::LogEditorWindow::AddLog(const TypeAlias::Math::Color& a_textColor, const std::string& a_text)
{
	// 空モッジの場合は追加するものがないため終了する
	if (a_text.empty()) { return; }

	// 現在のTextBufferサイズ
	// この位置から今回の新しいログが追加される
	int l_oldSize = m_textBuffer.size();

	// 直前のログが改行で終わっている場合
	// m_textLineOffsetsの最後 == 現在のTextuBufferサイズとなり、
	// 「まだ何も書かれていない次の行」を表している
	// その行へ今回のログを書き始めるため
	// その行の色を今回指定された色へ変更する
	if (m_textLineOffsets.Size > NULL &&
		!m_textLineColorList.empty())
	{
		const int l_lastLineIndex = m_textLineOffsets.Size - k_nextLineIndexOffset;

		if (m_textLineOffsets[l_lastLineIndex] == l_oldSize)
		{
			m_textLineColorList[static_cast<std::size_t>(l_lastLineIndex)] = a_textColor;
		}
	}

	// EditorManager側ですでに完成しているログ文字r津を
	// ImGuiTextBufferへそのまま追加する
	// append()へ終端位置も渡しているので
	// std::string内の指定範囲だけを正確に追加できる
	m_textBuffer.append(a_text.data(), a_text.data() + a_text.size());

	// 今回追加されてた文字列の範囲だけ走査して、
	// 新しい行の開始位置を記録する
	for (const int l_newSize = m_textBuffer.size(); l_oldSize < l_newSize; ++l_oldSize)
	{
		if (m_textBuffer[l_oldSize] != '\n') { continue; }

		// 改行の次の位置が次行の開始位置
		m_textLineOffsets.push_back(l_oldSize + k_nextLineStartOffset);

		// 今回追加した一つのログは、
		// 複数行になっていてもすべて同じ色として扱う
		m_textLineColorList.push_back(a_textColor);
	}

	// 自動スクロールが有効なら
	// 次回DrawLog()時に一番下まで移動する
	if (m_canAutoScroll)
	{
		m_canScrollToBottom = true;
	}
}

void FWK::Editor::LogEditorWindow::DrawEditorOptions()
{
	if (ImGui::BeginPopup(k_optionString.data()))
	{
		// 自動スクロールするかどうかを決めるチェックボタン
		if (ImGui::Checkbox(k_optionAutoScrollString.data(), &m_canAutoScroll))
		{
			m_canScrollToBottom = false;
		}

		ImGui::EndPopup();
	}

	if (ImGui::Button(k_optionString.data()))
	{
		ImGui::OpenPopup(k_optionString.data());
	}

	ImGui::SameLine();

	// ログのクリア
	if (ImGui::Button(k_optionClearString.data()))
	{
		ClearLog();
	}

	ImGui::SameLine();

	// ログのコピー
	if (ImGui::Button(k_optionCopyString.data()))
	{
		ImGui::LogToClipboard();
	}

	ImGui::SameLine  ();
	m_textFilter.Draw(k_optionFilterString.data(), k_filterInputWidth);

	const ImVec2 l_size = {};

	ImGui::BeginChild(k_optionScrollString.data(),
					  l_size, 
					  false,
					  ImGuiWindowFlags_HorizontalScrollbar);
}

void FWK::Editor::LogEditorWindow::DrawLog()
{
	// アイテムの間隔を"0"にする
	const ImVec2 l_offset = {};

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing , l_offset);

	// テキストバッファーからログ全体の先端と終端を取得
	const auto* l_buffer    = m_textBuffer.begin();
	const auto* l_bufferEnd = m_textBuffer.end  ();

	// フィルターが有効かどうかを確認
	if (m_textFilter.IsActive())
	{
		for (int l_lineNUM = 0; l_lineNUM < m_textLineOffsets.Size; ++l_lineNUM)
		{
			const char* l_lineStart = l_buffer + m_textLineOffsets[l_lineNUM];
			const char* l_lineEnd   = (l_lineNUM + k_nextLineIndexOffset < m_textLineOffsets.Size) ? (l_buffer + m_textLineOffsets[l_lineNUM + k_nextLineIndexOffset] + k_excludeNewLineOffset) : l_bufferEnd;

			// フィルター条件に合わない行はスキップ
			if (!m_textFilter.PassFilter(l_lineStart , l_lineEnd)) { continue; }

			const auto& l_textColor = m_textLineColorList[static_cast<std::size_t>(l_lineNUM)];

			// FWK側ではDirectX::SimpleMath::Colorを使用しているため
			// ImGuiへ渡す直前だけImVec4へ変換する
			const ImVec4& l_imGuiTextColor = { l_textColor.R(),
											   l_textColor.G(),
											   l_textColor.B(),
											   l_textColor.A() };

			ImGui::PushStyleColor(ImGuiCol_Text, l_imGuiTextColor);

			// 合致する行だけ表示
			ImGui::TextUnformatted(l_lineStart, l_lineEnd);
			ImGui::PopStyleColor  ();
		}
	}
	else
	{
		// ユーザーが見ている部分だけ描画するクリッパー
		ImGuiListClipper l_clipper = {};

		l_clipper.Begin(m_textLineOffsets.Size);

		while (l_clipper.Step())
		{
			for (int l_lineNUM = l_clipper.DisplayStart; l_lineNUM < l_clipper.DisplayEnd; l_lineNUM++)
			{
				const char* l_lineStart = l_buffer + m_textLineOffsets[l_lineNUM];
				const char* l_lineEnd   = (l_lineNUM + k_nextLineIndexOffset < m_textLineOffsets.Size) ? (l_buffer + m_textLineOffsets[l_lineNUM + k_nextLineIndexOffset] + k_excludeNewLineOffset) : l_bufferEnd;

				const auto& l_textColor = m_textLineColorList[static_cast<std::size_t>(l_lineNUM)];

				const ImVec4& l_imGuiTextColor = { l_textColor.R(),
											       l_textColor.G(),
											       l_textColor.B(),
											       l_textColor.A() };

				ImGui::PushStyleColor (ImGuiCol_Text, l_imGuiTextColor);
				ImGui::TextUnformatted(l_lineStart,   l_lineEnd);
				ImGui::PopStyleColor  ();
			}
		}

		l_clipper.End();
	}

	ImGui::PopStyleVar();

	if (m_canScrollToBottom)
	{
		ImGui::SetScrollHereY(k_scrollToBottomRatio);

		m_canScrollToBottom = false;
	}

	ImGui::EndChild();
}

void FWK::Editor::LogEditorWindow::ClearLog()
{
	m_textBuffer.clear       ();
	m_textLineOffsets.clear  ();
	m_textLineColorList.clear();

	// 一行目の開始位置を保証するためにpush_back
	m_textLineOffsets.push_back(k_ensureFirstLine);

	m_textLineColorList.emplace_back(Constant::k_debugDefaultColor);
}