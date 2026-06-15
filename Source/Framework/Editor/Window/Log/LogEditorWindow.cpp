#include "LogEditorWindow.h"

void FWK::Editor::LogEditorWindow::INIT()
{
	// 自動スクロールをONにしておきログをすべてクリア
	m_canAutoScroll     = k_enabelCanAutoScroll;
	m_canScrollToBottom = k_enabelCanScrollToBottom;

	ClearLog();
}

void FWK::Editor::LogEditorWindow::Draw()
{
	DrawEditorOptions();
	DrawLog          ();
}

void FWK::Editor::LogEditorWindow::AddLog(const char* a_format, ...)
{
	// 現在のm_textBufferのサイズを保存(どこから新しい文字列が始まるかを記録)
	int l_oldSize = m_textBuffer.size();

	// va_listを使って可変引数から文字列を追加
	va_list l_args;

	va_start(l_args, a_format);

	// フォーマット済み文字列をバッファに追加
	m_textBuffer.appendfv(a_format, l_args);

	va_end(l_args);

	// 新たに追加されて部分を走査し、改行文字がある位置を次の行の開始位置として記録
	for (int l_newSize = m_textBuffer.size(); l_oldSize < l_newSize; l_oldSize++)
	{
		// 改行を検出したら、次の行の先頭インデックスを記録
		if (m_textBuffer[l_oldSize] != '\n') { continue; }

		m_textLineOffsets.push_back(l_oldSize + k_nextLineStartOffset);
	}

	// 自動スクロールが有効な場合、視界描画時に最下部へスクロールするフラグをセット
	if (m_canAutoScroll)
	{
		m_canScrollToBottom = true;
	}
}

void FWK::Editor::LogEditorWindow::DrawEditorOptions()
{
	if (!ImGui::Begin("LogView"))
	{
		ImGui::End();
		return;
	}

	if (ImGui::BeginPopup("Options"))
	{
		// 自動スクロールするかどうかを決めるチェックボタン
		if (ImGui::Checkbox("AutoScroll" , &m_canAutoScroll))
		{
			m_canScrollToBottom = false;
		}

		ImGui::EndPopup();
	}

	if (ImGui::Button("Options"))
	{
		ImGui::OpenPopup("Options");
	}

	ImGui::SameLine();

	// ログのクリア
	if (ImGui::Button("Clear"))
	{
		ClearLog();
	}

	ImGui::SameLine();

	// ログのコピー
	if (ImGui::Button("Copy"))
	{
		ImGui::LogToClipboard();
	}

	ImGui::SameLine();
	m_textFilter.Draw("Filter" , k_filterInputWidth);

	const ImVec2 l_size = {};

	ImGui::BeginChild("Scrolling"                          , 
					  l_size                               , 
					  false                                ,
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
			ImGui::TextUnformatted(l_lineStart , l_lineEnd);
		}
	}
	else
	{
		// ユーザーが見ている部分だけ描画するクリッパー
		ImGuiListClipper l_clipper;
		l_clipper.Begin(m_textLineOffsets.Size);

		while (l_clipper.Step())
		{
			for (int l_lineNum = l_clipper.DisplayStart; l_lineNum < l_clipper.DisplayEnd; l_lineNum++)
			{
				const char* l_lineStart = l_buffer + m_textLineOffsets[l_lineNum];
				const char* l_lineEnd   = (l_lineNum + k_nextLineIndexOffset < m_textLineOffsets.Size) ? (l_buffer + m_textLineOffsets[l_lineNum + k_nextLineIndexOffset] + k_excludeNewLineOffset) : l_bufferEnd;

				ImGui::TextUnformatted(l_lineStart , l_lineEnd);
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
	ImGui::End     ();
}

void FWK::Editor::LogEditorWindow::ClearLog()
{
	m_textBuffer.clear     ();
	m_textLineOffsets.clear();

	// 一行目の開始位置を保証するために"push_back"
	m_textLineOffsets.push_back(k_ensureFirstLine);
}