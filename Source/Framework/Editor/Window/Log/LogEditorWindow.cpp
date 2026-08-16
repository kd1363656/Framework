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

	ImGui::SameLine();
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
		for (int l_lineNum = 0; l_lineNum < m_textLineOffsets.Size; l_lineNum++)
		{
			const char* l_lineStart = l_buffer + m_textLineOffsets[l_lineNum];
			const char* l_lineEnd   = (l_lineNum + k_nextLineIndexOffset < m_textLineOffsets.Size) ? (l_buffer + m_textLineOffsets[l_lineNum + k_nextLineIndexOffset] + k_excludeNewLineOffset) : l_bufferEnd;

			// フィルター条件に合わない行はスキップ
			if (!m_textFilter.PassFilter(l_lineStart , l_lineEnd)) { continue; }

			// 合致する行だけ表示
			ImGui::Text(l_lineStart , l_lineEnd);
		}
	}
	else
	{
		// ユーザーが見ている部分だけ描画するクリッパー
		ImGuiListClipper l_clipper = {};

		l_clipper.Begin(m_textLineOffsets.Size);

		while (l_clipper.Step())
		{
			for (int l_lineNum = l_clipper.DisplayStart; l_lineNum < l_clipper.DisplayEnd; l_lineNum++)
			{
				const char* l_lineStart = l_buffer + m_textLineOffsets[l_lineNum];
				const char* l_lineEnd   = (l_lineNum + k_nextLineIndexOffset < m_textLineOffsets.Size) ? (l_buffer + m_textLineOffsets[l_lineNum + k_nextLineIndexOffset] + k_excludeNewLineOffset) : l_bufferEnd;

				ImGui::Text(l_lineStart , l_lineEnd);
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
	m_textBuffer.clear     ();
	m_textLineOffsets.clear();

	// 一行目の開始位置を保証するためにpush_back
	m_textLineOffsets.push_back(k_ensureFirstLine);
}